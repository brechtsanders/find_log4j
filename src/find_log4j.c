#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <limits.h>
#endif
#include <dirtrav.h>

#define FILE_EXT ".txt"

#if defined(_MSC_VER) || (defined(__MINGW32__) && !defined(__MINGW64__))
#define strcasecmp _stricmp
#define strcasestr _stristr
#elif defined(_WIN32)
static char* strcasestr (const char* haystack, const char* needle)
{
  size_t needlelen;
  if (!needle || !*needle)
      return (char*)haystack;
  needlelen = strlen(needle);
  while (*haystack) {
    if (_strnicmp(haystack, needle, needlelen) == 0)
      return (char*)haystack;
    haystack++;
  }
  return NULL;
}
#endif

struct config_struct {
  uint64_t count;
  FILE* dst;
};

int file_found (dirtrav_entry info)
{
  //printf("%s\n", info->fullpath); return 0;/////
  struct config_struct* config = (struct config_struct*)info->callbackdata;
  const char* ext = dirtrav_prop_get_extension(info);
  if (ext && strcasecmp(ext, ".jar") == 0) {
    if (strcasestr(dirtrav_prop_get_name(info), "log4j-") != NULL) {
      config->count++;
      fprintf(config->dst, "%s\n", info->fullpath);
    }
  }
  return 0;
}

#ifdef _WIN32
#define folder_found NULL
#else
int folder_found (dirtrav_entry info)
{
  if (strcmp(info->fullpath, "/dev/") == 0 || strcmp(info->fullpath, "/sys/") == 0 || strcmp(info->fullpath, "/proc/") == 0)
    return -1;
  return 0;
}
#endif

int drive_found (dirtrav_entry info)
{
  //printf("%s\n", info->fullpath); return 0;/////
  return dirtrav_traverse_directory(info->fullpath, file_found, folder_found, NULL, info->callbackdata);
}

int main(int argc, char *argv[])
{
  char* dstfilename;
  struct config_struct config = {
    .count = 0,
    .dst = stdout
  };

  //get destination filename
  {
    char* hostname = NULL;
    size_t pos = strlen(argv[0]);
#ifdef _WIN32
    DWORD hostnamelen = 0;
    GetComputerNameExA(ComputerNamePhysicalNetBIOS, NULL, &hostnamelen);
    if (hostnamelen) {
      if ((hostname = (char*)malloc(hostnamelen)) != NULL) {
        if (!GetComputerNameExA(ComputerNamePhysicalNetBIOS, hostname, &hostnamelen)) {
          free(hostname);
          hostname = NULL;
        }
      }
    }
#else
#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#endif
    char hostname_buf[HOST_NAME_MAX];
    if (gethostname(hostname_buf, sizeof(hostname_buf)) == 0) {
      hostname = strdup(hostname_buf);
    }
#endif
    if (!hostname) {
      fprintf(stderr, "Unable to get computer name\n");
      return 1;
    }
    while (pos > 0 && argv[0][pos - 1] != '/'
#ifdef _WIN32
      && argv[0][pos - 1] != '\\'
#endif
    )
      pos--;
    dstfilename = (char*)malloc(pos + strlen(hostname) + strlen(FILE_EXT) + 1);
    memcpy(dstfilename, argv[0], pos);
    strcpy(dstfilename + pos, hostname);
    strcpy(dstfilename + pos + strlen(hostname), FILE_EXT);
  }
  //create output file
  if ((config.dst = fopen(dstfilename, "wb")) == NULL) {
    fprintf(stderr, "Error creating output file: %s\n", dstfilename);
    return 2;
  }

  //try to get elevated access
  dirtrav_elevate_access();

  //scan disk(s)
  dirtrav_iterate_roots(drive_found, &config);

  //report number of files found
  printf("Files found: %" PRIu64 "\n", config.count);

  //clean up
  fclose(config.dst);
  return 0;
}

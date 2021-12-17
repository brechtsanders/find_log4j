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
#include <miniargv.h>
#include <dirtrav.h>
#include <multifinder.h>
#include "find_log4j_version.h"

#define PROGRAM_NAME "find_log4j"
#define PROGRAM_DESC "Tool to search for Apache Log4j Security Vulnerabilities CVE-2021-45046 and CVE-2021-44228"
#define FILE_EXT ".txt"

#define READBUFFERSIZE (32 * 1024)

#if defined(_MSC_VER) || (defined(__MINGW32__) && !defined(__MINGW64__))
#define strcasecmp _stricmp
//#define strcasestr _stristr
#endif
#ifdef _WIN32
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

//configuration data structure
struct config_struct {
  uint64_t count;
  FILE* dst;
  int simplesearch;
  multifinder finder;
  const char* currentpath;
  char* readbuffer;
};

//get position after directory path (including trailing path separator)
size_t get_path_length_from_path (const char* path)
{
    size_t pos = strlen(path);
    while (pos > 0 && path[pos - 1] != '/'
#ifdef _WIN32
      && path[pos - 1] != '\\'
#endif
    )
      pos--;
    return pos;
}

//callback function for when match is found in file contents
int data_found (const char* data, size_t datalen, void* patterncallbackdata, void* callbackdata)
{
  struct config_struct* config = (struct config_struct*)callbackdata;
  config->count++;
  fprintf(config->dst, "%s\n", config->currentpath);
  return 1;
}

//callback function called for each file encountered during file system traversal
int file_found (dirtrav_entry info)
{
  //printf("%s\n", info->fullpath); return 0;/////
  struct config_struct* config = (struct config_struct*)info->callbackdata;
  const char* ext = dirtrav_prop_get_extension(info);
  if (config->simplesearch) {
    if (ext && strcasecmp(ext, ".jar") == 0) {
      if (strcasestr(dirtrav_prop_get_name(info), "log4j-") != NULL) {
        config->count++;
        fprintf(config->dst, "%s\n", info->fullpath);
      }
    }
  } else {
    if (ext && (strcasecmp(ext, ".jar") == 0 || strcasecmp(ext, ".ear") == 0 || strcasecmp(ext, ".war") == 0)) {
      FILE* src;
      if ((src = fopen(info->fullpath, "rb")) != NULL) {
        size_t buflen;
        config->currentpath = info->fullpath;
        while ((buflen = fread(config->readbuffer, 1, READBUFFERSIZE, src)) > 0) {
          multifinder_process(config->finder, config->readbuffer, buflen);
        }
        multifinder_finalize(config->finder);
        fclose(src);
      }
      multifinder_reset(config->finder);
    }
  }
  return 0;
}

//callback function called before each folder encountered during file system traversal
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

//callback function called for each command line argument
int scan_path (const miniargv_definition* argdef, const char* value, void* callbackdata)
{
  //printf("%s\n", value); return 0;/////
  return dirtrav_traverse_directory(value, file_found, folder_found, NULL, callbackdata);
}

//callback function called for each drive letter on Windows or / on other platforms
int scan_root (dirtrav_entry info)
{
  //printf("%s\n", info->fullpath); return 0;/////
  return dirtrav_traverse_directory(info->fullpath, file_found, folder_found, NULL, info->callbackdata);
}

int main (int argc, char *argv[], char *envp[])
{
  int showhelp = 0;
  int showversion = 0;
  int createparentpath = 0;
  int deleteifempty = 0;
  char* dstfilename = NULL;
  struct config_struct config = {
    .count = 0,
    .dst = stdout,
    .simplesearch = 0,
    .finder = NULL,
    .currentpath = NULL,
    .readbuffer = NULL
  };
  //definition of command line arguments
  const miniargv_definition argdef[] = {
    {'h', "help",            NULL,   miniargv_cb_increment_int, &showhelp,            "show command line help"},
    {'v', "version",         NULL,   miniargv_cb_increment_int, &showversion,         "show program version"},
    {'o', "output",          "FILE", miniargv_cb_strdup,        &dstfilename,         "file where to write output to (\"-\" for console)\nif not specified a file <hostname>.txt will be created in the same folder as the executable file"},
    {'p', "parent",          NULL,   miniargv_cb_increment_int, &createparentpath,    "create directory output file if it doesn't exist yet"},
    {'d', "delete",          NULL,   miniargv_cb_increment_int, &deleteifempty,       "delete output file if nothing was found"},
    {'s', "simple",          NULL,   miniargv_cb_increment_int, &config.simplesearch, "perform simple search (filenames matching \"*log4j-*.jar\")\nthe default is to search the contents of all \"*.jar\", \".ear\" and \".war\" files for \"JndiLookup.class\""},
    {0,   NULL,              "PATH", scan_path,                 NULL,                 "path(s) to scan (default is to scan all disks)"},
    {0, NULL, NULL, NULL, NULL, NULL}
  };
  //parse environment and command line flags
  if (miniargv_process_arg_flags(argv, argdef, NULL, NULL) != 0)
    return 1;
  //show help if requested or if no command line arguments were given
  if (showhelp /*|| argc <= 1*/) {
    printf(
      PROGRAM_NAME " - Version " FIND_LOG4J_VERSION_STRING " - " FIND_LOG4J_LICENSE " - " FIND_LOG4J_CREDITS "\n"
      PROGRAM_DESC "\n"
      "Usage: " PROGRAM_NAME " "
    );
    miniargv_arg_list(argdef, 1);
    printf("\n");
    miniargv_help(argdef, NULL, 26, 0);
    return 0;
  }
  //show version if requested
  if (showversion) {
    printf(PROGRAM_NAME " " FIND_LOG4J_VERSION_STRING "\n");
    return 0;
  }
  //set default output filename if none was given
  if (!dstfilename) {
    size_t pos;
    char* hostname = NULL;
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
    pos = get_path_length_from_path(argv[0]);
    dstfilename = (char*)malloc(pos + strlen(hostname) + strlen(FILE_EXT) + 1);
    memcpy(dstfilename, argv[0], pos);
    strcpy(dstfilename + pos, hostname);
    strcpy(dstfilename + pos + strlen(hostname), FILE_EXT);
  }
  //initialize search if needed
  if (!config.simplesearch) {
    if ((config.finder = multifinder_create(data_found, NULL, &config)) != NULL) {
      multifinder_add_pattern(config.finder, "JndiLookup.class", MULTIFIND_PATTERN_CASE_SENSITIVE, NULL);
      if ((config.readbuffer = (char*)malloc(READBUFFERSIZE)) == NULL) {
        fprintf(stderr, "Memory allocation error, falling back to simple search\n");
        config.simplesearch = 0;
      }
    } else {
      fprintf(stderr, "Error initializing search, falling back to simple search\n");
      config.simplesearch = 0;
    }
  }

  //try to get elevated access
  dirtrav_elevate_access();

  //open output file
  if (strcmp(dstfilename, "-") == 0) {
    config.dst = stdout;
  } else {
    if (createparentpath) {
      size_t parentpathlen = get_path_length_from_path(dstfilename);
      char* parentpath = strdup(dstfilename);
      parentpath[parentpathlen] = 0;
      dirtrav_make_full_path(NULL, parentpath, S_IRWXU | S_IRWXG | S_IRWXO);
      free(parentpath);
    }
    if ((config.dst = fopen(dstfilename, "wb")) == NULL) {
      fprintf(stderr, "Error creating output file: %s\n", dstfilename);
      return 2;
    }
  }

  if (miniargv_get_next_arg_param(0, argv, argdef, NULL) == 0) {
    //scan disk(s)
    dirtrav_iterate_roots(scan_root, &config);
  } else {
    //scan selected path(s)
    miniargv_process_arg_params(argv, argdef, NULL, &config);
  }

  //close output file
  fclose(config.dst);

  //clean output file if empty
  if (deleteifempty && config.count == 0 && strcmp(dstfilename, "-") != 0) {
    unlink(dstfilename);
  }

  //report number of files found
  printf("Files found: %" PRIu64 "\n", config.count);

  //clean up
  if (config.finder) {
    multifinder_free(config.finder);
    if (config.readbuffer)
      free(config.readbuffer);
  }
  return 0;
}

//Safe Log4J: Java 8: 2.16.0, Java 7: 2.12.2

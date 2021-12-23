find_log4j
==========
Searches all disks for log4j-*.jar files and writes the matching paths to <hostname>.txt in the same folder as the executable

Description
-----------
This command line application searches all disks for log4j-*.jar files and writes the matching paths to <hostname>.txt in the same folder as the executable.
This tool was needed when Apache Log4j Security Vulnerabilities [CVE-2021-45046](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2021-45046) and [CVE-2021-44228](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2021-44228) were exposed.

Goals
-----
- low footprint (small single executable file)
- low system requirements (even runs on Windows XP)
- portable, the code is cross-platform and can be built on Windows, macOS, Linux, and possible other platforms

Command line help
-----------------
```
find_log4j - Version 0.0.5 - MIT - Brecht Sanders (2021)
Tool to search for Apache Log4j Security Vulnerabilities CVE-2021-45046 and CVE-2021-44228
Usage: find_log4j [-h] [-v] [-o FILE] [-p] [-d] [-s] PATH
Command line arguments:
  -h, --help              show command line help
  -v, --version           show program version
  -o FILE, --output=FILE  file where to write output to ("-" for console)
                          if not specified a file <hostname>.txt will be
                          created in the same folder as the executable file
  -p, --parent            create directory output file if it doesn't exist yet
  -d, --delete            delete output file if nothing was found
  -s, --simple            perform simple search (filenames matching
                          "*log4j-*.jar")
                          the default is to search the contents of all "*.jar",
                          ".ear" and ".war" files for "JndiLookup.class"
  PATH                    path(s) to scan (default is to scan all disks)
```

Dependencies
------------
To build find_log4j from source the following dependencies are needed:
 - [miniargv](https://github.com/brechtsanders/miniargv)
 - [libdirtrav](https://github.com/brechtsanders/libdirtrav)
 - [libmultifinder](https://github.com/brechtsanders/libmultifinder)

Building from source
--------------------
First make sure the dependencies are installed.

Then run:
```
make
```

To install run:
```
make install
```
or to install in a specific location:
```
make install PREFIX=/usr/local
```

See also the [GitHub Actions workflow](.github/workflows/find_log4j.yml) for more information for a full build including dependencies.

License
-------
find_log4j is released under the terms of the MIT License (MIT), see LICENSE.txt.

This means you are free to use libdirtrav in any of your projects, from open source to commercial.

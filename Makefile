ifeq ($(OS),)
OS = $(shell uname -s)
endif
PREFIX = /usr/local
CC   = gcc
CPP  = g++
AR   = ar
LIBPREFIX = lib
LIBEXT = .a
ifeq ($(OS),Windows_NT)
BINEXT = .exe
SOEXT = .dll
else ifeq ($(OS),Darwin)
BINEXT =
SOEXT = .dylib
else
BINEXT =
SOEXT = .so
endif
INCS = -Iinclude
CFLAGS = $(INCS) -Os
CPPFLAGS = $(INCS) -Os
LIBS =
LDFLAGS =
ifeq ($(OS),Darwin)
STRIPFLAG =
else
STRIPFLAG = -s
endif
MKDIR = mkdir -p
RM = rm -f
RMDIR = rm -rf
CP = cp -f
CPDIR = cp -rf
DOXYGEN = $(shell which doxygen)
ifdef STATIC
CFLAGS += -DSTATIC
LDFLAGS += -static
endif

OSALIAS := $(OS)
ifeq ($(OS),Windows_NT)
ifneq (,$(findstring x86_64,$(shell gcc --version)))
OSALIAS := win64
else
OSALIAS := win32
endif
endif
PACKAGE_NAME := find_log4j

MINIARGV_LDFLAGS = -lminiargv
LIBDIRTRAV_LDFLAGS = -ldirtrav
LIBMULTIFINDER_LDFLAGS = -lmultifinder

ifneq ($(OS),Windows_NT)
SHARED_CFLAGS += -fPIC
endif

TOOLS_BIN = find_log4j$(BINEXT)

COMMON_PACKAGE_FILES = README.md LICENSE Changelog.txt
SOURCE_PACKAGE_FILES = $(COMMON_PACKAGE_FILES) Makefile src/*.h src/*.c build/*.cbp build/*.depend

all: $(TOOLS_BIN)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) 

find_log4j$(BINEXT): src/find_log4j.o
	$(CC) -o $@ $^ $(STRIPFLAG) $(MINIARGV_LDFLAGS) $(LIBDIRTRAV_LDFLAGS) $(LIBMULTIFINDER_LDFLAGS) $(LDFLAGS)

tools: $(TOOLS_BIN)

install: all
	mkdir -p $(PREFIX)/bin/
	$(CP) $(TOOLS_BIN) $(PREFIX)/bin/

version:
	sed -ne "s/^#define\s*FIND_LOG4J_VERSION_[A-Z]*\s*\([0-9]*\)\s*$$/\1./p" src/find_log4j_version.h | tr -d "\n" | sed -e "s/\.$$//" > version

.PHONY: package
package: version
	tar cfJ $(PACKAGE_NAME)-$(shell cat version).tar.xz --transform="s?^?$(PACKAGE_NAME)-$(shell cat version)/?" $(SOURCE_PACKAGE_FILES)

.PHONY: package
binarypackage: version
	$(MAKE) PREFIX=binpkg_$(OSALIAS)_temp install STATIC=1
ifneq ($(OS),Windows_NT)
	tar cfJ "$(PACKAGE_NAME)-$(shell cat version)-$(OSALIAS).tar.xz" --transform="s?^binpkg_$(OSALIAS)_temp/??" $(COMMON_PACKAGE_FILES) binpkg_$(OSALIAS)_temp/*
else
	rm -f $(PACKAGE_NAME)-$(shell cat version)-$(OSALIAS).zip
	cp -f $(COMMON_PACKAGE_FILES) binpkg_$(OSALIAS)_temp/
	cd binpkg_$(OSALIAS)_temp && zip -r -9 "../$(PACKAGE_NAME)-$(shell cat version)-binary-$(OSALIAS).zip" $(COMMON_PACKAGE_FILES) * && cd ..
endif
	rm -rf binpkg_$(OSALIAS)_temp

.PHONY: clean
clean:
	$(RM) src/*.o $(TOOLS_BIN) version

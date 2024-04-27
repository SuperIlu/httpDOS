###
# Makefile for cross compiling HttpDos for FreeDOS/MS-DOS
# All compilation was done with DJGPP 12.2.0 built from https://github.com/andrewwutw/build-djgpp
# ./build-djgpp.sh --target=i586-pc-msdosdjgpp --prefix=/home/ilu/cross all
# make sure the DJGPP toolchain is in your path (i586-pc-msdosdjgpp-XXX)!
###


THIRDPARTY	= 3rdparty
WATT32		= $(THIRDPARTY)/Watt-32
INI			= $(THIRDPARTY)/ini-20220806/src
MBEDTLS		= $(THIRDPARTY)/mbedtls-2.28.3

LIB_WATT	= $(WATT32)/lib/libwatt.a
LIB_MBEDTLS = $(MBEDTLS)/library/libmbedtls.a

# compiler
CDEF     = -DENABLE_STATIC_FILE=1 -DCERT_FILES=1 -DLINUX=0
CFLAGS   = -MMD -Wall -std=gnu99 -O2 -march=i386 -mtune=i586 -ffast-math -fomit-frame-pointer $(INCLUDES) -fgnu89-inline -Wmissing-prototypes $(CDEF)
INCLUDES = \
	-I$(realpath ./src) \
	-I$(realpath $(WATT32))/inc \
	-I$(realpath $(MBEDTLS))/include \
	-I$(realpath $(INI))/

# linker
LIBS     = -lmbedtls -lmbedx509 -lmbedcrypto -lwatt -lm -lemu 
LDFLAGS  = -s \
	-L$(WATT32)/lib \
	-L$(MBEDTLS)/library/

# output
EXE      = HTTPDOS.exe
RELZIP   = httpdos-X.Y.Z.zip

# dirs/files
DOJSPATH		= $(realpath .)
BUILDDIR		= build

CROSS=i586-pc-msdosdjgpp
CROSS_PLATFORM=i586-pc-msdosdjgpp-
CC=$(CROSS_PLATFORM)gcc
CXX=$(CROSS_PLATFORM)g++
AR=$(CROSS_PLATFORM)ar
LD=$(CROSS_PLATFORM)ld
STRIP=$(CROSS_PLATFORM)strip
RANLIB=$(CROSS_PLATFORM)ranlib
DXE3GEN = dxe3gen
DXE3RES = dxe3res
export

MPARA=-j8

PARTS= \
	$(BUILDDIR)/main.o \
	$(BUILDDIR)/middleware.o \
	$(BUILDDIR)/server.o \
	$(BUILDDIR)/ini/ini.o

all: httpdos

httpdos: init libwatt32 libmbedtls $(EXE)

libmbedtls: $(LIB_MBEDTLS)
$(LIB_MBEDTLS):
	$(MAKE) $(MPARA) -C $(MBEDTLS) -f Makefile lib

libwatt32: $(LIB_WATT)
$(LIB_WATT):
	DJ_PREFIX=$(dir $(shell which $(CC))) $(MAKE) $(MPARA) -C $(WATT32)/src -f djgpp.mak

$(EXE): $(PARTS) init libwatt32 libmbedtls
	$(CC) $(LDFLAGS) -o $@ $(PARTS) $(LIBS)

$(BUILDDIR)/%.o: src/%.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/ini/%.o: $(INI)/%.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

cacert.pem:
	curl --remote-name --time-cond cacert.pem https://curl.se/ca/cacert.pem

init:
	mkdir -p $(BUILDDIR) $(BUILDDIR)/ini

clean:
	rm -rf $(BUILDDIR)/
	rm -f $(EXE) cacert.pem W32DHCP.TMP

distclean: clean wattclean mbedtlsclean

wattclean:
	$(MAKE) -C $(WATT32)/src -f djgpp.mak clean

mbedtlsclean:
	$(MAKE) -C $(MBEDTLS) -f Makefile clean

zip:
	rm -f $(RELZIP)
	zip -9 -r $(RELZIP) $(EXE) HTTPDOS.INI WATTCP.CFG CWSDPMI.EXE *.md rootCA.* server.* static/

.PHONY: clean distclean init zip

DEPS := $(wildcard $(BUILDDIR)/*.d)
ifneq ($(DEPS),)
include $(DEPS)
endif

#
#  Makefile for Waterloo TCP sample applications
#
#  Watcom386/Causeway executables.
#

MODEL   = flat
CC      = *wcc386 -3r
CFLAGS  = -bt=dos -mf -oaxt -DWATT32_STATIC
LFLAGS  = system causeway option stack=50k
LIBRARY = library ../lib/wattcpwf.lib

BUILD_MESSAGE = Watcom386/Causeway binaries done

!include wccommon.mak

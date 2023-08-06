#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or 
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

# Makefile for NOISE.SYS Version 0.6.1 accessory utilities
# written for bcc 4, tasm 3.2 or 4.1, exe2bin

LINK = tlink

COPT = -3 -mc -O2 -Z
# or use -mt option for tiny model (.com files)

.asm.obj:
  $(AS) $(AOPT) $(AFLAGS) $&.asm

.obj.exe:
  $(LINK) $(LOPT) $&.obj

all: sample stirfile

sample: sample.exe
stirfile: stirfile.exe

sample.exe: sample.c usuals.h
  $(CC) $(COPT) $(CFLAGS) $&.c

stirfile.exe: stirfile.c randpool.c md5.c randpool.h md5.h usuals.h
  $(CC) $(COPT) $(CFLAGS) stirfile.c randpool.c md5.c

clean:
  erase *.exe
  erase *.obj
  erase *.map

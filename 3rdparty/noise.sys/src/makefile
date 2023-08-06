#  Copyright (c) 1995, 1996, 2004, Robert Rothenburg Walking-Owl.
#  (Portions by Colin Plumb.)  All rights reserved.
# 
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
# 
#    * Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
# 
#    * Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in
#      the documentation and/or other materials provided with the
#      distribution.
# 
#    * Neither the names of the authors nor the names of its
#      contributors may be used to endorse or promote products derived
#      from this software without specific prior written permission.
# 
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
#  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
#  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
#  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
#  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
#  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
#  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
#  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#  POSSIBILITY OF SUCH DAMAGE.
# 

# Makefile for NOISE.SYS Version 0.6.3a1
# written for bcc 4, tasm 3.2 or 4.1, exe2bin

# Make optional defines (ie, "make -DLIST"):
#       -DEXE           Build EXE-format driver rather than core image
#       -DLIST          Generate a listing
#       -DCPU={3,4,5}   Build for CPU (ie, -DCPU=4 for i486)
#       -DWIN95         Build to be Win95-friendly
#       -DDEBUG         Build for testing entropy sources (no hashing/mixing)
#       -DTSC           Use CPU Timestamp Counter (if CPU={4,5} only)

# A note about Win95/MS-DOS 7 and long filenames:
#       If the long filenames cause problems with older versions of make,
#       use the following in a short batch file before making:
#
#               move initnoise.inc initnois.inc
#               move initnois.inc initnoise.inc
#               move multiplex.inc multiple.inc
#               move multiple.inc multiplex.inc
#

# Note: assumes AS, AFLAGS, CC, CFLAGS macros are already defined in
#       "builtins.mak" (Borland make.exe); otherwise define them
#       below, with requisite paths for includes, libraries, etc.

LINK = tlink
LOPT = /k

MBIN = exe2bin

COPT = -3 -mt -O2 -Z

!if $d(DEBUG)
DBG=1
!else
DBG=0
!endif

!if $d(CPU)
!else
CPU=3
!endif

AOPT = /d__DEBUG=$(DBG) /d__CPU=$(CPU)

!if $d(WIN95)
AOPT = $(AOPT) /dWIN95
!endif

!if $d(TSC)
AOPT = $(AOPT) /d__UseTSC=1
!else
AOPT = $(AOPT) /d__UseTSC=0
!endif

!if $d(LIST)
AOPT = $(AOPT) /l
!endif

.asm.obj:
  $(AS) $(AOPT) $(AFLAGS) $&.asm

.obj.exe:
  $(LINK) $(LOPT) $&.obj

.exe.sys:
!if $d(EXE)
  copy $&.exe $&.sys
!else
  $(MBIN) $&.exe $&.sys
!endif
#  if exist $&.exe erase $&.exe

# (Will the above line work for execom also?)

noise.sys: noise.obj

noise.obj: noise.asm noise.def noise.mac \
           sha.inc multiplex.inc \
           initnoise.inc printf.inc cpuid.inc

clean:
  if exist noise.lst erase noise.lst
  if exist noise.sys erase noise.sys
  if exist noise.exe erase noise.exe
  erase *.obj
  erase *.map

COMMENT |
== License =================================================================

  Copyright (c) 1995, 1996, 2004, Robert Rothenburg Walking-Owl.
  (Portions by Colin Plumb.)  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the
      distribution.

    * Neither the names of the authors nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.

===========================================================================|

        include printf.inc

; --------------------------------------------------------------------------
rInitDevice     PROC    NEAR
                les     bx, DWORD PTR rrh ; es:[bx] -> random response hdr
                lea     dx, EndOfDevice
                mov     WORD PTR es:[bx].ReqHdr.Addr, dx
                mov     WORD PTR es:[bx].ReqHdr.Addr+2, cs
                jmp     rFinished
rInitDevice     ENDP
; Note: Win95/MS-DOS 7 needs a proper value returned here.

; --------------------------------------------------------------------------
; Notes: assumes es:[di] -> urandom request hdr
InitDevice	PROC	NEAR
;;  ifdef WIN95
;;                mov     WORD PTR [STATAPI+2], cs
;;  endif
		mov	ax, 1600h	; Check	if in Windows
; We probably don't need this, but it's	better to be on	the safe side
		int	MltplxSvc
		test	al, al		; if al	!= 0, set Windows flag
		jz	SHORT @InDe0
		or	WORD PTR Flags,	WindowsFlag
@InDe0:

		call	DriverInit

		les	bx, DWORD PTR urh ; es:[bx] -> urandom response	hdr
		lea	dx, EndOfDevice
		mov	WORD PTR es:[bx].ReqHdr.Addr, dx
		mov	WORD PTR es:[bx].ReqHdr.Addr+2,	cs
		jmp	Finished
InitDevice	ENDP


; --------------------------------------------------------------------------
; -----	hook interrupts	and print a small message
;	assumes	al=int,	bx=routine (save oldvec	at bx-4)

@HookListPtr    LABEL   WORD
                DW      OFFSET @HookList
Install_Int	PROC	NEAR
		pushf		; save vector number and flags
		xor	ah, ah
		push	ax
		cli		; disable interrupt flag

                mov     di, [@HookListPtr]
                mov     BYTE PTR [di], al
                mov     WORD PTR [di+1], bx
                add     WORD PTR [@HookListPtr], 3

		push	ax
		push	bx
		mov	ah, 35h	; get interrupt	vector and save	it
		int	DOS
		pop	ax
		xchg	ax, bx
                mov     WORD PTR [bx+4], es
                mov     WORD PTR [bx+2], ax
		mov	dx, bx
		pop	ax
		mov	ah, 25h	; set new vector
		int	DOS
                test    WORD PTR UserOptionFlags, SuppressTechie
                jnz     SHORT @iiNoTechie
		Printf	<OFFSET	@iitext>
		xor	ax, ax		  ; ax == 0, no	error
		popf			  ; restore flags
		ret
@iiRejected:
		mov	ax, -1		  ; ax == -1, an error occurred
		add	sp, 2		  ; restore stack, flags
		popf
		ret
@iiNoTechie:
		xor	ax, ax
		add	sp, 2		  ; restore stack, flags
		popf
		ret
Install_Int	ENDP

@iitext	LABEL	BYTE
	DB	' %2x',0

; --------------------------------------------------------------------------
; We first want	to set flags for selected options, do any diagnostics later

ShowLicenseFlag	equ 8000h  ; Print license when installing
SuppressTechie  equ 4000h  ; Show interrupts and other stuff when load
InitPoolFlag	equ 2000h  ; Initialize	entropy	pool when installing
SampleDiskFlag  equ 1000h  ; Install Int 13h handler (v0.5)
SampleVideoFlag equ 0800h  ; Enable video sampling
MouseFlag       equ 0400h  ; Enable mouse sampling
DriftFlag       equ 0200h  ; Enable drift sampling
;;SuppressInt13   equ 0100h  ; Don't hook Int 13h (Win95/DOS7)

FlagSets PROC NEAR
; /A = enable sampling of Audio	card
@EnableAudio:
  ife __samplaudio
	jmp  @DisabledOption
  else
	and  WORD PTR Flags, (not NoAudioFlag)
	clc
	ret
; <----	Allow user to specify base address of sound card
  endif
; /C = enable sampling of Clock	drift
@EnableDrift:
  ife __sampldrift
	jmp  @DisabledOption
  else
        or   [UserOptionFlags], DriftFlag        
	clc
	ret
  endif
; /I = Initialize entropy pool with random keystrokes
@InitializeOnInstall:
  ife __sample09
	jmp  @DisabledOption
  else
        or   [UserOptionFlags], InitPoolFlag
        mov  [SeedCount], (InitSeedCnt shl FRACBITS)
; <----	Allow user to specify the number of bits to sample?
	clc
	ret
  endif
; /K = Keep entropy pool topped	off after hashing
@EnableKeepFull:
	jmp  @DisabledOption
; /L = show License
@ShowLicense:
	or   UserOptionFlags, ShowLicenseFlag
	clc
	ret
; /M = enable sampling of mouse	Movement
@EnableMotion:
  ife __samplmouse
	jmp  @DisabledOption
  else
        xor     ax, ax                  ; test if mouse is installed
        int     33h
        test    ax, ax
        jnz     SHORT @mousOk0
        mov     al, 'M'                 ;
        jmp     @UnableOption
@mousOk0:
        or   UserOptionFlags, MouseFlag
	clc
	ret
  endif
; /D = Install Int 13h sampler
@InstallDisk:
  ife __sample13
        jmp  @DisabledOption
  else

; Note: a call to Int 2F/AX=4A33h should return AX==0 if MS-DOS 7,
;       AX != 0 for anything else.

        cmp  [DosVersionMaj], 7    ; Check DOS version
        jge  @UnableOption
; <---- Instead print a different error message...
        or   UserOptionFlags, SampleDiskFlag
        clc
        ret
  endif
@Quiet:
        or   [UserOptionFlags], SuppressTechie
        clc
        ret
; /V = Video
@SampleVideoRetrace:
  ife __SamplVideo
        jmp  @DisabledOption
  else
        or   UserOptionFlags, SampleVideoFlag
        clc
        ret
  endif
; /O = Ignore OS version
@IgnoreOSVersion:
        mov    [DosVersionMaj], 0
        clc
        ret
@BadOption:
        MPrintf <ax OFFSET msgIllegalOption>
        xor    ah, ah   ; wait for keypress
        int    Keyboard
	stc
	ret
msgIllegalOption LABEL BYTE
        DB LF, "I do not recognize that option ('%c'). Aborting installation.", CR, LF
	DB "Allowable options are:", CR, LF
  if __samplaudio
	DB " /A",TAB,"enable sampling of Audio card", CR, LF
  endif
  if __sampldrift eq 1
	DB " /C",TAB,"enable sampling of Clock drift during idle time",	CR, LF
  endif
  if __sample13
        DB " /D",TAB,"enable sampling of Disk access (disabled under DOS 7+)", CR, LF
  endif
  if __sample09
	DB " /I",TAB,"Initialize entropy pool with keystrokes when installing",	CR, LF
  endif
	DB " /L",TAB,"show License", CR, LF
        DB " /O",TAB,"ignore OS version", CR, LF
        DB " /Q",TAB,"quiet installation (suppress techie messages)", CR, LF
  if __samplmouse
	DB " /M",TAB,"enable sampling of Mouse movement", CR, LF
  ;; during idle-time or periodic timer
  endif
  if __SamplVideo
        DB " /V",TAB,"Sample vertical retrace intervals from Video", CR, LF
  endif
  DB LF
msgPressKey LABEL BYTE
        DB "Press a key to continue....", BELL, CR, LF
        DB LF, 0
@UnableOption:
        MPrintf <ax OFFSET msgUnableOption>
        jmp     SHORT @waitOnKey
@DisabledOption:
        MPrintf <ax OFFSET msgDisabledOption>
@waitOnKey:
        MPrintf <OFFSET msgPressKey>
        xor    ah, ah
        int    Keyboard
	stc
	ret
msgDisabledOption:
	DB LF, "Sorry, that option ('%c') is not available in this build."
        DB CR, LF, LF, 0
msgUnableOption:
        DB LF, "Sorry, unable to use that option ('%c') on this system."
        DB CR, LF, LF, 0
FlagSets ENDP
UserOptionFlags	LABEL WORD
  DW 0
ArgTable LABEL BYTE
  DB 'ACIKLMDVQO'
NumArgs	equ $ -	ArgTable
ArgHandlr LABEL	DWORD
  DW @EnableAudio
  DW @EnableDrift
  DW @InitializeOnInstall
  DW @EnableKeepFull
  DW @ShowLicense
  DW @EnableMotion
  DW @InstallDisk
  DW @SampleVideoRetrace
  DW @Quiet
  DW @IgnoreOSVersion
; --------------------------------------------------------------------------
DoOption PROC	NEAR ; al = character to check
; MS-DOS 5.0 will convert to uppercase from CONFIG.SYS,	but some dyanmic
;   loader utilities will not convert the command line options.
	cmp	al, 'a'
	jb	SHORT @doNotLower
	cmp	al, 'z'
	ja	SHORT @doNotLower
	sub	al, 20h
@doNotLower:
        mov     bx, NumArgs-1
@doLoop:
	cmp	al, ArgTable[bx]
	je	SHORT @doCheck
	dec	bx
	jns	SHORT @doLoop
	jmp	@BadOption
@doCheck:
	add	bx, bx
	lea	bx, ArgHandlr[bx]
	jmp	[bx]
DoOption ENDP

; --------------------------------------------------------------------------
DriverInit	PROC	NEAR	; Initialize the driver
		Printf	<OFFSET	InitMsg> ; Print header
  if (__CPU)
		call	TestCPU		  ; Check if CPU is Ok to use
		test	ax, ax
		jz	SHORT CorrectCPU
		ret			  ; if not, return(-1)
  CorrectCPU:
  else
    %out **Wisecrack: I'd hate to see how slow this runs on an 8088...
  endif
                mov     ah, 30h           ; Get DOS version
                int     DOS
                mov     [DosVersionMaj], al

                mov     ax, cs
                mov     Word Ptr [NoiseWin].TSR_PSP_Segment, ax
                mov     Word Ptr [NoiseWin].TSR_ID_Block+2, ax
                mov     Word Ptr [NoiseWin].TSR_ID_Block, Offset NoiseID

; -----	check for command-line arguments passed	to driver (redone v0.4.3)
                 les     di, DWORD PTR es:[di+18]
;@diFirstArg:    mov     al, es:[di]
;                cmp     al, ' '      ; find first space
;                je      SHORT @diArgumentLoop
;                cmp     al, CR       ; end of line?
;                je      SHORT @diArgEOL ; done...
;                inc     di
;                jmp     SHORT @diFirstArg
@diArgumentLoop:
		inc	di	     ; increment pointer
		mov	al, es:[di]  ; get character
		cmp	al, CR	     ; end of line?
		je	SHORT @diArgEOL	; done...
                cmp     al, LF       ; OS/2 DOS boxes sometimes end line w/LF
                je      SHORT @diArgEOL
		cmp	al, '/'	     ; SwitchChar?
		jne	SHORT @diArgumentLoop ;	if not,	continue...
		inc	di	     ; increment
		mov	al, es:[di]  ; get character
		cmp	al, CR	     ; end of line? then ignore...
		je	SHORT @diArgEOL
                cmp     al, LF
                je      SHORT @diArgEOL
		call	DoOption     ; check option, set any flags...
		jnc	SHORT @diArgumentLoop ;	if no error, continue...
		mov	ax, -1	     ; return(-1)
		ret
@diArgEOL:
		test	WORD PTR UserOptionFlags, ShowLicenseFlag
		jz	SHORT @diNoShowLic
                MPrintf <OFFSET LicenseText>
@diNoShowLic:
                test    WORD PTR UserOptionFlags, SuppressTechie
                jnz     SHORT @diNoTechie
		Printf	<OFFSET	rheader	cs OFFSET StartInstall>
@diNoTechie:
; --------------------------------------------------------------------------
; Might be nifty to use a simple cipher or PRNG to fill the the bytes in
; the random and output pools.  See the comment in the TimerLatch routine.

  if __sample15
                InstallSampler 15h, _Multitask
  endif
		InstallSampler MltplxSvc, _MultiplexServ
  if __sample09
		InstallSampler 9, _keysample
  endif
  if __sample13
                cmp  [DosVersionMaj], 7
                jge  SHORT @NoDiskI
		InstallSampler 13h, _dsksample
                test [UserOptionFlags], SampleDiskFlag
                jz   SHORT @NoDiskI
                or   [Flags], flSamplDisk
  @NoDiskI:
  endif
  if  __sample08
                InstallSampler 1Ch, _timerlatch ; was Int 08 -v0.4.9
  endif
  if __sample28
		InstallSampler 28h, _idlewait
  endif
  if __SamplExec or __DosSpinner
                InstallSampler 21h, _NewInt21
  endif
; --------------------------------------------------------------------------
; This should be modified to search for free interrupts...

        cmp     BYTE PTR [HookAPI], 2Dh ; find a free AMIS function
        jne     SHORT @notAMIS
; if might be better to set AL to [HookFunk] and search upwards until we
; get back to the value in [HookFunc] again, then return an error...
        xor     ax, ax
@chkAMIS:
        int     2Dh
        test    al, al
        jz      SHORT @freeAMIS
; (!) <---- check if AL=FFh, if so, no more free frunctions!
        inc     al
        jmp     SHORT @chkAMIS
@freeAMIS:
        mov     BYTE PTR [HookFunc], al
@notAMIS:
; (!) The API should always be the LAST interrupt hook added!
        InstallSampler <BYTE PTR [HookAPI]>, NoiseHook
        mov     [FarCallAddr], cs

    Printf  <OFFSET NewLine>
    xor  ax, ax
    mov  bx, ax
    mov  al, [HookAPI]
    mov  bl, [HookFunc]
    Printf <bx ax OFFSET msgAPI>

  if __SamplDrift
        test    [UserOptionFlags], DriftFlag
        jz      SHORT @NoDrift1
        or      [Flags], flSamplDrift
        Printf  <OFFSET msgDrift>
@NoDrift1:
  endif
  if __SamplVideo
        test    [UserOptionFlags], SampleVideoFlag
        jz      SHORT @NoVid1
        ; <---- We should test if proper video card exists
        or      [Flags], flSamplVideo
        Printf  <OFFSET msgVideo>
@NoVid1:
  endif
  if __Sample13
        test    [UserOptionFlags], SampleDiskFlag
        jz      SHORT @NoDsk1
        Printf  <OFFSET msgDisk>
@NoDsk1:
  endif
  if __SamplMouse
        test    [UserOptionFlags], MouseFlag
        jz      SHORT @NoMouse1
;        xor     ax, ax                  ; test if mouse is installed
;        int     33h
;        test    ax, ax
;        jz      SHORT @NoMouse1         ; (*) we should print an error
        or      [Flags], flSamplMouse
        Printf  <OFFSET msgMouse>
@NoMouse1:
  endif
  if __samplaudio
        test    WORD PTR Flags, NoAudioFlag
        jnz     SHORT @diNoAudioMsg
        Printf  <sbPort OFFSET AudioMsg>
@diNoAudioMsg:
  endif
        Printf  <OFFSET msgMethods>

  if (initseedcnt ne 0) and (__sample09) ;; initialize pool w/keystrks
; --------------------------------------------------------------------------
		mov	ax, WORD PTR SeedCount ; seedcount set to 0?
		test	ax, ax
                jz      SHORT @NoSample
		Printf	<OFFSET	SeedMessage>	 ; if not, sample...
@Seedlings:
		mov	ax, SeedCount	       ; get count
		sub	ax, FreshCount	       ; compate to entropy (v0.4.3)
                js      SHORT @Seeded          ; if count==0, done
                mov     bx, OutQueue
                shl     bx, 3
  if FRACBITS
		shr	ax, FRACBITS
  endif
                sub     ax, bx
                js      SHORT @Seeded
		Printf	<ax OFFSET DigitCount> ; output	bits needed
		xor	ah, ah		       ; get keyboard character
		int	Keyboard

		cmp	ax, WORD PTR LastCode  ; ignore	if triplets
		je	SHORT @IgnoreCode
		cmp	ax, WORD PTR LastCode+2
		je	SHORT @IgnoreCode
		shl	DWORD PTR LastCode, 16 ; set last keys pressed
		mov	WORD PTR LastCode, ax
                call    TrackDeltas            ; add code to pool
@IgnoreCode:

		jmp	SHORT @Seedlings
@Seeded:
		Printf	<OFFSET	OkMessage>
@NoSample:
  endif
                call    Sample ; add installation time (won't hurt)
		xor	ax, ax	     ; return 0	= ok
		ret
DriverInit	endp

  if initseedcnt
SeedCount LABEL	WORD
    DW	0
LastCode LABEL WORD
    DW	2 DUP (?)
SeedMessage LABEL BYTE
    DB CR
    DB "We are going to seed the entropy pool with some truly random bits by", CR, LF
    DB "timing your keystrokes.  Please type until the counter reaches zero: ", 0
DigitCount LABEL BYTE
    DB "%4d", 4	DUP (BackSpace), 0
OkMessage LABEL	BYTE
    DB 6 DUP (' '), CR, LF, LF, "That's enough. Thank you.", CR, LF, 0
  endif


msgMethods LABEL BYTE
  if __Sample09
    DB 254, " Sampling keystroke timings", CR, LF
  endif
  if (__SamplExec) and (__Sample21 eq 0)
    DB 254, " Sampling DOS exec() and exit() timings", CR, LF
  endif
  if (__SamplFlush) and (__Sample21 eq 0)
    DB 254, " Sampling DOS flush() timings", CR, LF
  endif
  if (__DosSpinner) or (__Sample21)
    DB 254, " Sampling access to DOS services (Int 21h)", CR, LF
  endif
  if (__MixAtTicks)
    DB 254, " Timer will be sampled during clock ticks", CR, LF
  endif
    DB LF, 0
  if __SamplDrift
msgDrift LABEL BYTE
  DB 254, " Sampling clock drift", CR, LF, 0
  endif
  if __SamplVideo
msgVideo LABEL BYTE
    DB 254, " Sampling vertical retrace intervals", CR, LF, 0
  endif
  if __Sample13
msgDisk LABEL BYTE
    DB 254, " Sampling disk access timings", CR, LF, 0
  endif
  ifdef __SamplMouse
msgMouse LABEL BYTE
    DB 254, " Sampling mouse movement and button-presses", CR, LF, 0
  endif
  if __samplaudio
AudioMsg LABEL BYTE
    DB 254, " Sampling audio card port %x", CR, LF, 0
  endif

msgAPI LABEL BYTE
    DB 254, " API installed at Int %x, Function %x", CR, LF, 0

include cpuid.inc    ; TestCPU routine

; --------------------------------------------------------------------------
InitMsg LABEL BYTE  ; ----- Initial text messages and other stuff
        DB CR, LF, DrvName, " Version "
        DB ZChr+((NoiseVersion/256) and 15), DOT
        DB ZChr+((NoiseVersion and 255) shr 4), DOT
        DB ZChr+(NoiseVersion and 15), BetaChar
        DB " Build ", ??date, " ", ??time
        DB CR, LF
        DB "by Robert Rothenburg Walking-Owl. Portions by Colin Plumb."
        DB CR, LF, "Copyright (C) 1995, 1996, 2004. All Rights Reserved.", CR, LF
        DB 0
; -----	This section is	for displaying warnings	when debug options enabled
NewLine	LABEL BYTE
  if (NOHASH+NOMIX)
        db  CR, LF, BELL, 254, " WARNING: "
  endif
  if NOHASH
        db "Hashing is disabled. "
  endif
  if NOMIX
	db "Mixing is disabled.	"
  endif
; -----	End warnings -----
        db CR, LF, 0
StartInstall LABEL   BYTE
        DB LF, 254, " Loaded at %4x:%4x "
	DB "Hooking interrupts:", 0
; --------------------------------------------------------------------------
LicenseText: 
     DB	LF
     DB '      Copyright (c) 1995, 1996, 2004, Robert Rothenburg Walking-Owl.', CR, LF
     DB '      (Portions by Colin Plumb.)  All rights reserved.', CR, LF
     DB '    ', CR, LF
     DB '      Redistribution and use in source and binary forms, with or without', CR, LF
     DB '      modification, are permitted provided that the following conditions', CR, LF
     DB '      are met:', CR, LF
     DB '    ', CR, LF
     DB '        * Redistributions of source code must retain the above copyright', CR, LF
     DB '          notice, this list of conditions and the following disclaimer.', CR, LF
     DB '    ', CR, LF
     DB '        * Redistributions in binary form must reproduce the above copyright', CR, LF
     DB '          notice, this list of conditions and the following disclaimer in', CR, LF
     DB '          the documentation and/or other materials provided with the', CR, LF
     DB '          distribution.', CR, LF
     DB '    ', CR, LF
     DB '        * Neither the names of the authors nor the names of its', CR, LF
     DB '          contributors may be used to endorse or promote products derived', CR, LF
     DB '          from this software without specific prior written permission.', CR, LF
     DB '    ', CR, LF
     DB '      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS', CR, LF
     DB '      "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT', CR, LF
     DB '      LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS', CR, LF
     DB '      FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE', CR, LF
     DB '      COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,', CR, LF
     DB '      INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,', CR, LF
     DB '      BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;', CR, LF
     DB '      LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER', CR, LF
     DB '      CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT', CR, LF
     DB '      LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN', CR, LF
     DB '      ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE', CR, LF
     DB '      POSSIBILITY OF SUCH DAMAGE.', CR, LF
     DB	0

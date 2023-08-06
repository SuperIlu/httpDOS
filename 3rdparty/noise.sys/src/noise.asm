COMMENT |
============================================================================

  NOISE.SYS v0.6.3a1, A random-noise device driver (25 Sep 2004)
  by Robert Rothenburg Walking-Owl.  Portions by Colin Plumb.
  Copyright (C)	1995, 1996, 2004. All Rights Reserved.

  This is code for a character device which samples various sources of
  system noise and mixes them into a pool which is mixed further using
  the Secure Hash Algorithm (SHA-1) transformation.

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
NoiseVersion = 0063h

include noise.def       ; definitions, assembler options, etc.
include noise.mac       ; assorted macros, other definitions

; --------------------------------------------------------------------------
_TEXT	segment	word public use16 'CODE'
        assume  cs:_TEXT, ds:_TEXT
	org	0
	start:
rheader LABEL   DWORD                   ; /dev/random device header
  if __ALIASRAND ; allow /dev/rand as an alias for /dev/random
        dw      alrheader, -1           ; device header offset
        dw      attribute+IOCTLSpt      ; character device
	dw	rstrategy
	dw	rinterrupt
        db      'RAND    '              ; device name
alrheader LABEL DWORD
  endif
        DW      uheader, -1             ; device header offset
        DW      attribute+IOCTLSpt      ; character device
        DW      rstrategy
        DW      rinterrupt
        DB      "RANDOM$ "              ; device name

; --------------------------------------------------------------------------
uheader LABEL   DWORD                   ; /dev/urandom device header
        DD      -1                      ; device header offset
        DW      attribute               ; character device
        DW      ustrategy
        DW      uinterrupt
        DB      "URANDOM$"              ; device name

rrh     LABEL   DWORD                   ; request header pointer
        DD      ?
urh     LABEL   DWORD                   ; request header pointer
        DD      ?

; We could probably get away with sharing request headers, but it's
; technically not good proctive to do so.)

; --------------------------------------------------------------------------
; Miscellaneous data used by the driver (see also the ControlOut area)

partial LABEL WORD      ; Used for entropy estimation
        DW 8000h
  if __SamplDrift
@TickCount      LABEL   DWORD
        DD      100000  ; 100,000 microseconds
  endif
Index   LABEL   WORD
        DW      0                       ; where new data is put
BytesAvail LABEL WORD                   ; hash output pointer
  ife NOHASH
        DW      0
StateSize EQU (hashbits/8)              ; 160 bits
InitState label dword                   ; initial SHA chaining variables
	dd 67452301h, 0EFCDAB89h, 98BADCFEh, 10325476h,	0C3D2E1F0h
State   label   dword                   ; initial SHA chaining variables
	dd 67452301h, 0EFCDAB89h, 98BADCFEh, 10325476h,	0C3D2E1F0h
Pool    DD      80 dup (?)              ; sampling pool SHA expansion array
  else
StateSize equ RawSize
        DW      rawsize-1
Pool    DD      16 dup (?)
  endif
  if POOLSIZE
OutIndex LABEL  WORD
        DW      0
OutPool LABEL DWORD
        DD POOLSIZE dup (?)
  endif
  if __samplmouse     ; delta values needed by mouse sampler
LastX   LABEL   WORD
        DW      0
DeltaX  LABEL   WORD
        DW      0
LastY   LABEL   WORD
        DW      0
DeltaY  LABEL   WORD
        DW      0
LastPos LABEL   WORD
        DW      0
  endif
; --------------------------------------------------------------------------
rdevicetable label word			; /dev/random device table
;        dw      rdevok                  ; Initialize driver (unused here)
        dw      rInitDevice
	dw	rdevok			; Media	Check
	dw	rcunknown
        dw      IOCTL_Read              ; IOCTL Read
	dw	rdevread		; Read
	dw	rcunknown		; Nondestructive Read (Removed 0.3.5)
        dw      rdevok ; DevStatus      ; Input Status
        dw      rdevok                  ; Flush input buffers
  if __AllowAddAPI
        dw      RDevWrite               ; Write
        dw      RDevWrite               ; Write with verify
        dw      rdevok                  ; Output status
        dw      rdevok                  ; Flush output buffers
  endif
rMAXCMD	equ	($ - rdevicetable) / 2	; Maximum allowable command
; --------------------------------------------------------------------------
udevicetable label word			; /dev/urandom device table
        dw      InitDevice              ; Initialize driver
	dw	devok			; Media	Check
	dw	cunknown
	dw	cunknown
	dw	devread			; Read
	dw	cunknown		; Nondestructive Read
	dw	devok			; Input	Status
        dw      devok
  if __AllowAddAPI
        dw      DevWrite                ; Write
        dw      DevWrite                ; Write with verify
        dw      rdevok                  ; Output status
        dw      rdevok                  ; Flush output buffers
  endif
uMAXCMD	equ	($ - udevicetable) / 2	; Maximum allowable command
; --------------------------------------------------------------------------
rstrategy       PROC    FAR
		mov	WORD PTR cs:rrh, bx
		mov	WORD PTR cs:rrh+2, es
		ret
rstrategy       ENDP

ustrategy       PROC    FAR
		mov	WORD PTR cs:urh, bx
		mov	WORD PTR cs:urh+2, es
		ret
ustrategy       ENDP

; We could probably get away with one strategy routine in DOS, but
; it gives me a bad vibe, and probably isn't good for multitasking
; --------------------------------------------------------------------------
rinterrupt      PROC    FAR
		cld
		SaveRegs
                inc     [UsageFlag]
		les	di, DWORD PTR rrh
		mov	bl, es:[di].ReqHdr.CmdCode
		xor	bh, bh
		cmp	bx, rMAXCMD
		ja	SHORT rcunknown
		add	bx, bx
		jmp	WORD PTR rdevicetable[bx]
  rcunknown:			       ; ----- Unknown command
		mov	ax, unknowncmd
		jmp	SHORT rfinished
  rdevok:				; Everything went Ok
		xor	ax, ax
  rfinished:				; Leave	the driver
		les	bx, DWORD PTR rrh ; es:[bx] -> response	header
		or	ax, DONE    ; set to done code
		mov	es:[bx].ReqHdr.StatusCode, ax
                dec     [UsageFlag]
		RestoreRegs
		ret
rinterrupt      ENDP
; --------------------------------------------------------------------------
uinterrupt      PROC    FAR
		cld
		SaveRegs
                inc     [UsageFlag]
		les	di, DWORD PTR urh
		mov	bl, es:[di].ReqHdr.CmdCode
		xor	bh, bh
		cmp	bx, uMAXCMD
		ja	SHORT cunknown
		add	bx, bx
		jmp	WORD PTR udevicetable[bx]
  cunknown:			       ; ----- Unknown command
		mov	ax, unknowncmd
		jmp	SHORT finished
; --------------------------------------------------------------------------
; initdevice routine moved to disposed portion of driver (v0.3.5)

  devok:				; Everything went Ok
		xor	ax, ax
  finished:				; Leave	the driver
		les	bx, DWORD PTR urh ; es:[bx] -> response	header
		or	ax, DONE    ; set to done code
		mov	es:[bx].ReqHdr.StatusCode, ax
                dec     [UsageFlag]
		RestoreRegs
		ret
uinterrupt      ENDP
; --------------------------------------------------------------------------
DoneRead        PROC    NEAR            ; Read operation completed
		mov	bytesavail, 0	; throw	away remaining bytes
		jmp	SHORT devok
DoneRead        ENDP
; --------------------------------------------------------------------------
DevStatus       PROC    NEAR
; This is needed if we intend to limit the amount of bytes returned
		cmp	freshcount, entropythreshold
		ja	SHORT somentpres ; is enough entropy present?
RDevBusy:
		mov	ax, BUSY
		jmp	SHORT rfinished
  somentpres:
		jmp	SHORT rdevok
DevStatus       endp
; --------------------------------------------------------------------------
; This expects to be called with ds == cs and es:[si] -> response header
RDevRead        PROC    NEAR            ; Read /dev/random
                cmp     [UsageFlag], 1
                ja      SHORT @ReadNothing
@DeviceClear:
  if POOLSIZE                           ; Read bytes from output pool
                mov     cx, OutQueue    ; check if pool has any bytes
                test    cx, cx
                jnz     SHORT @AboveThresh0
@ReadNothing:
		mov	es:[di].ReqHdr.Siz, 0  ; if not, return	nothing
                jmp     SHORT RDevBusy
@AboveThresh0:        
  else                                  ; Read bytes from hash
                mov     cx, FreshCount  ; check if pool has entropy
                cmp     cx, EntropyThreshold
                ja      SHORT @AboveThresh1
@ReadNothing:
		mov	es:[di].ReqHdr.Siz, 0  ; if not, return	nothing
                jmp     SHORT  RDevBusy
@AboveThresh1:
		shr	cx, 3+FRACBITS	       ; bytes requested > entropy?
  endif
                cmp     cx, es:[di].ReqHdr.Siz ; Bytes to read
                jnb     SHORT @EnoughEntropy
		mov	es:[di].ReqHdr.Siz, cx ; if so,	limit output
@EnoughEntropy:
                mov     cx, es:[di].ReqHdr.Siz 
  if POOLSIZE
                sub     OutQueue, cx
                mov     bx, [OutIndex]
		les	di, es:[di].ReqHdr.Addr	 ; Address to read to
@PoolCopyLoop:
; It's more secure to wipe the contents of the OutPool[] after reading
; from it, but if you're not paranoid you may want to preserve the pool
; for further mixing. (Apparently this is NOT the cause of problems in
; MS-DOS 7/Win95 with the 'copy' command and reads from /dev/random)
   if 1
                xor     al, al ; (Was: mov al, bl in v0.5.7)
                xchg    al, BYTE PTR OutPool[bx]
   else
                mov     al, BYTE PTR OutPool[bx]
   endif
                dec     bx
  if 0
                jns     SHORT @rdr00
                mov     bx, (POOLSIZE*4)-1
@rdr00:
  else
                and     bx, (POOLSIZE*4)-1
  endif
                stosb
                loop    @PoolCopyLoop
                mov     [OutIndex], bx
  else
                mov     bx, [BytesAvail]
		les	di, es:[di].ReqHdr.Addr	 ; Address to read to
		call	read
                mov     [BytesAvail], 0 ; throw away remaining bytes
  endif
		jmp	rdevok
RDevRead        ENDP
; --------------------------------------------------------------------------
devread         PROC    NEAR                    ; Read /dev/urandom
		mov	cx, es:[di].ReqHdr.Siz	; Bytes	to read
                mov     bx, [BytesAvail]
                les     di, es:[di].ReqHdr.Addr ; Address to read to

DO_READ         LABEL
; (!) Save return address and flow directly into read() routine
                push    OFFSET DoneRead
devread         ENDP

; --------------------------------------------------------------------------
; The read() routine rewritten by Colin Plumb (v0.3)  Some changes made
;       since by Robert Walking-Owl.
; Assumes:  cx == bytes to read, bx == available bytes, es:[di] -> buffer

Read            PROC    NEAR
                inc     [UsageFlag]

; (!) To-do: move [UsageFlag] handling to strategy routine

  readloop:
		dec	bx
		jns	SHORT copybyte
		mov	ax, WORD PTR freshcount
		sub	ax, (statesize*8) shl FRACBITS
		jns	SHORT freshleft
		xor	ax, ax
  freshleft:	xchg	ax, WORD PTR freshcount

  ife NOHASH
		push	cx		; Oops,	we need	to hash	more data
		call	SHATransform
; -----	Now add	the hash output	back to	the pool.
;	This cheaply stirs up the pool even more.
		mov	cx, statesize/2
		mov	si, OFFSET state
  recycle:
		lodsw
                call    Accumulate
		loop	recycle
; <---- Removed FillWithNoise
		pop	cx		; And restore the pointers
  endif
		mov	bx, statesize-1

  copybyte:	; -----	Copy bytes to user buffer
  ife NOHASH
		mov	al, BYTE PTR initstate[bx] ; (changed v0.3.5)
		xchg	al, BYTE PTR state[bx]
		; -----	Don't keep a copy of random data!
  else
		mov	al, BYTE PTR pool[bx]
  endif
		stosb
		loop	readloop
                dec     [UsageFlag]
		ret
Read            ENDP

; --------------------------------------------------------------------------
; Add samples to driver through device_write()
  if __AllowAddAPI
DevWrite        PROC    NEAR
                mov     ax, Offset Accumulate
                call    device_write
                jmp     DevOk
DevWrite        ENDP

RDevWrite       PROC    NEAR
                mov     ax, Offset TrackDeltas
                call    device_write
                jmp     RDevOk
RDevWrite       ENDP

device_write    PROC    NEAR
  if 1 ; ----- Set to '0' to turn devices into bit-buckets
                mov     WORD PTR [SamplingFunc], ax
                push    es
                mov     cx, es:[di].ReqHdr.Siz
                les     di, es:[di].ReqHdr.Addr
@dw00:          mov     al, BYTE PTR es:[di]
                xor     ah, ah
                push    es di cx
                call    WORD PTR [SamplingFunc]
                pop     cx di es
                inc     di
                loop    @dw00
                pop     es
  endif
                ret
SamplingFunc    LABEL   WORD
                DW      Offset Accumulate
device_write    ENDP
  endif

  ife NOHASH
		include	sha.inc		; -----	Secure Hash Transform -----
  else
        %out    **Warning: The random pool will not be hashed!
  endif

  if __sample08
; --------------------------------------------------------------------------
; We don't sample the timer here, rather we use	the timer to trigger
; periodic samplings of	other things...

  if SamplePeriod gt 1
CycleCounter	LABEL	WORD
                DW      SamplePeriod
  endif
                ibm_isp _timerlatch
		pushf
                call    DWORD PTR cs:[_timerlatch+2]
  if SamplePeriod gt 1
                dec     WORD PTR cs:[CycleCounter]   ; adjust counter
		js	SHORT ItsTime		     ; if < 0, deal with it
		iret
  endif
  ItsTime:	pushad
		push	ds	; save registers
                mov     ax, cs
                mov     ds, ax
  if SamplePeriod gt 1
                mov     WORD PTR CycleCounter, SamplePeriod
  endif

; <---- Possibly run a good PRNG or cipher to garble Pool[] and OutPool[]?

; <---- One could sample the mouse here insteead of during the Idle.
 if (__DosSpinner) and (__Sample21 eq 0)  ; Sample the counter
                xor     ax, ax
                xchg    ax, @DosSpinner                
  if NOMIX
                test    ax, ax
                jz      @T00
  elseif __MixAtTicks
; It might be better to use SampleTimerWord macro and then call Accumulate
; to mix the output queue and not bother estimating entropy of the sample.
                call    Sample
  endif
                cmp     ax, @LastDosSpin
                je      @T00
                mov     @LastDosSpin, ax
                call    TrackDeltas
@T00:
 endif
; <----- Add code to sample Calnet/Newbridge or similar RNG cards here?

		pop	ds
		popad
		iret
  endif

  if __Sample28
; --------------------------------------------------------------------------
  if __SamplVideo
;@VideoSpin      LABEL   WORD ; Counter used in sampling
;                DW      0
@LastRetrace    LABEL   BYTE ; Value of the last sample
                DB      0
  endif
  if __Sampldrift
@IdleSpinner    LABEL   WORD
                DW      0
@LastSpin       LABEL   WORD
                DW      0
@SpinFlag       LABEL   BYTE
                DB      80h
  endif
                ibm_isp _idlewait, Reset_timerlatch
                pushf
                call    DWORD PTR cs:[_idlewait+2]
		pushad
		push	ds
		mov	ax, cs
		mov	ds, ax

; (!) There may be correlations in drift or vsync sampling, so use with
;     caution, and set __FULLHASH to 1.  The results may be better if
;     sampling the Pentium time-stamp counter though.

  if __SamplVideo
; (!) Tested on a Cirrus Logic SVGA card. May not work for EGA systems.

                test    [Flags], flSamplVideo
                jz      SHORT @NoRetrace
;                inc     [@VideoSpin]
                mov     dx, 03DAh        ; Read status port
                in      al, dx
                mov     bl, al           ; Get the last read
                xchg    bl, [@LastRetrace]
                test    al, 08h          ; vsync/retrace bit set?
                jz      SHORT @NoRetrace ; if not, don't sample
                test    bl, 08h          ; was the last test also a vsync?
                jnz     SHORT @NoRetrace ; if so, don't sample
                call    Sample
; -----
;                xor     ax, ax           ; Sample spinner...
;                xchg    ax, [@VideoSpin]
;                call    TrackDeltas
@NoRetrace:
  endif
  if __Sampldrift
                inc     @IdleSpinner     ; increment the spinner
                test    BYTE PTR @SpinFlag, 81h
                jz      SHORT @@00       ; don't sample if alarm didn't go
                test    [Flags], WindowsFlag
                jnz     SHORT @@00       ; don't sample if in Windows
                test    [Flags], flSamplDrift
                jz      SHORT @@00
                mov     ax, @IdleSpinner ; get sample
  if 1
                cmp     ax, @LastSpin    ; ignore if the same as last
                je      SHORT @@01
                mov     @LastSpin, ax
  endif
                call    TrackDeltas      ; add sample
@@01:
                mov     dx, WORD PTR @TickCount   ; set a new alarm
                mov     cx, WORD PTR @TickCount+2 ; tickcount microseconds
                mov     bx, OFFSET @SpinFlag
                mov     ax, 8300h
  if (__CPU lt 3)
                mov     BYTE PTR @SpinFlag, al
  endif
                push    es cs
                pop     es
  if __Sample15
                pushf
                push    cs
                call    @mt15Exit
  else
                int     15h
  endif
  if (__CPU ge 3)
                setc    @SpinFlag
  else
                jnc     SHORT @@02
                mov     @SpinFLag, 80h ; reset flag if unable to set timer
  endif
@@02:
                pop     es
@@00:
  endif
  if __samplmouse eq 1
		SampleMousePos
  endif
;; ----- Got rid of SampleAudio	for now... (v0.4.2)
;;		  SampleAudio

		pop	ds
		popad
		iret
  endif

  if __sample09
; --------------------------------------------------------------------------
; Handler for the keyboard interrupt (Int 09h, IRQ1)

; A possible modification would be to accumulate the scancode as well(?)
; it may be of little value, and possibly a detriment to the randomness

                ibm_isp _keysample
                pushf               ; Call previous handler first
                call    DWORD PTR cs:[_keysample+2]
                cli                 ; disabled NMIs
		pushad		    ; save registers
		push	ds
                call	Sample	    ; accumulate sample
		pop	ds	    ; restore registers
		popad
		iret

  endif
  if __sample13
                ibm_isp _DskSample, Reset_keysample
; ----- if loaded after the disk cache, then the entropy is negligable.
                test    cs:[Flags], flSamplDisk
                jz      SHORT @dsNoSample
                pushf
                cli
                pushad              ; save registers, set ds == cs
                push    ds
                call    Sample
                pop     ds          ; restore registers
                popad
                popf
@dsNoSample:    jmp     DWORD PTR cs:[_DskSample+2] ; call original handler
  endif

	include	multiplex.inc

; --------------------------------------------------------------------------
; Trashes ax, bx and cx, sets ds = cs. Leaves everything else alone.
Sample  LABEL   NEAR
        mov     ax, cs      ; set ds = cs
        mov     ds, ax
; ----- Removed option to call the Windows timer, since it wasn't necessary
        SampleTimerWord     ; Get 16-bit sample from timer0
  if (__CPU ge 4) and (__UseTSC) and (__Sample32Bits)
        push    ax          ; for i586, gather 32-bit samples from TSC
        rol     eax, 16
        call    TrackDeltas
        pop     ax
  endif

TrackDeltas LABEL NEAR      ; Estimate the entropy of given samples
;        pushf
;        cli
  ife POOLSIZE
        cmp     FreshCount, 512 shl FRACBITS ; if maxed out, don't estimate
        je      Accumulate
  endif
        mov     cx, ax
        sub     cx, [LastTick] ; cx = delta
  if 0                         ; cx = abs(cx), supposedly a "faster" way of
        cwd                    ;        means of calculating abs() function
        xor     cx, dx         ;        since it doesn't use branches...
        sub     cx, dx         ;
  else
        jns     SHORT @tdPos   ; cx = abs(cx)
        neg     cx
@tdPos:
  endif
        mov     [LastTick], ax ; save last tick
; <---- Removed the delta = delta ^ (delta<<<8) mixing

; Since we don't have a table for 64k samples, we hash the last N samples
; down into a smaller table.  This tends to underestimate the entropy(?).
; --------------------------------------------------------------------------
; Rather than hashing we use a checksum of the last N-samples (xor'd). This
; seems to do well to limit the estimated entropy of repetative and cyclic
; samples, although it needs to be stuffed with N repeated bytes.

  if N ge 2
        xor     [CurrentHash], cx
        mov     bx, [indexNtable]
        xchg    cx, lastNtable[bx]
        sub     bx,2               ; Fixed bug v0.5.2: lastNTable[] words!
        jns     SHORT @td00
        mov     bx, (N-1)*2
@td00:
        mov     [indexNtable], bx
        xor     [CurrentHash], cx
        mov     bx, [CurrentHash]

; --------------------------------------------------------------------------
; The older (<=v0.5.1) hashing method.  It doesn't do so well for some forms
; of cyclic data, though it might be improved with careful tweaking.

;        mov     bx, [indexNtable]
;        mov     dx, lastNtable[bx]
;        mov     lastNtable[bx], cx
;        sub     bx, 2
;        jns     @td00
;        mov     bx, (N-1)*2
;@td00:  mov     [indexNtable], bx
;        rol     dx, N*5
;        xor     cx, dx
;        xor     cx, [CurrentHash]
;        rol     cx, 5
;        mov     [CurrentHash], cx
;        mov     bx, cx
  else
        mov     bx, cx
  endif

UpdateTable LABEL NEAR
  if TABLESIZE eq 256  ; Probably not a good idea...
        xor     bl, bh
        xor     bh, bh
  else
        and     bx, (TABLESIZE-1)*2    ; Mask off index bits
  endif
        mov     cx, WORD PTR [Counter] ; We only care about low 16-bits
        inc     DWORD PTR [Counter]
        xchg    cx, deltatable[bx]
        sub     cx, deltatable[bx]     ; Use counter delta (v0.5.2 bug fix)
        neg     cx
CalcEntropy LABEL NEAR
        mov     bp, ax          ; Save ax

;if N ge 2
        cmp     cx, N           ; no entropy if delta<N
        jna     @ceNoEntropy
;else
;        test    cx, (not 1)     ; no entropy if bits 1-15 are 0
;        jz      @ceNoEntropy    ; <-- or set bx=0 and jmp UpdateEstimate?
;endif

;
; For some reason using the i386 bsr instruction is slower then a loop that
; tests and shifts (at least on a 486DX4).  It might be faster on a 586+ to
; use the following between the cutlines:
;
;       mov     ax, cx
;       bsr     cx, ax
;       mov     bx, cx
;;      inc     bx              ; bx = cx+1 if you'll accept 16 bit samples
;       neg     cl              ; ax = ax << (15-cl)
;       add     cl, 15
;       shl     ax, cl
; -----
        mov     bx, 15          ; bx = 15 (or 15+1 if you'll accept 16 bits)
@ceDeltaLoop:
        test    cx, cx          ; if bit 15 set on cx, break
        js      SHORT @ce00
        add     cx, cx          ; cx = cx << 1 (add faster than shl)
        dec     bx              ; bx = bx - 1
        jnz     SHORT @ceDeltaLoop ; if bx != 0, continue
@ce00:
        mov     ax, cx          ; ax = cx
; -----
        xor     dx, dx
        mul     WORD PTR partial
        mov     cx, dx          ;
        test    cx, cx
        js      SHORT @ce01
        add     cx, cx
        dec     bx        
@ce01:
        mov     partial, cx
  if FRACBITS
        shl     bx, FRACBITS
CalcFracEntropy:
        mov     ax, cx
        mov     cx, (1 shl FRACBITS)-1
@ceFracLoop:
        xor     dx, dx
        mul     ax
        mov     ax, dx
        test    ax, ax
        js      SHORT @ce02
        add     bx, cx
        jmp     SHORT @ce03
@ce02:
        add     ax, ax
@ce03:
        shr     cx, 1
        jnz     SHORT @ceFracLoop
  endif
; --------------------------------------------------------------------------
; Update the entropy estimate
; --------------------------------------------------------------------------
; assumes bx = (bits<<FRACBITS); bp = sample

UpdateEstimate LABEL NEAR
        sub     bx, log2N              ; log2(diff)-log2(N)
  if __API and (POOLSIZE eq 0)
  ; allows API to return entropy for sample (meant for debugging)
        mov     dx, bx
  endif
        add     bx, freshcount
        js      @ceNoEntropy           ; added v0.5
        cmp     bx, (512 shl FRACBITS)        
        jc      SHORT @ceStoreFresh
        mov     bx, (512 shl FRACBITS) ; Carry clear: bx >= (512<<FRACBITS)
@ceStoreFresh:
        mov     FreshCount, bx
; --------------------------------------------------------------------------
; Update the output pool
; --------------------------------------------------------------------------
; assumes bx = [FreshCount]; bp = sample

; if [FreshCount] >= [Quality] (or 512 bits if __FULLHASH) then [Quality]
; bytes will be mixed into a larger entropy pool used as an output buffer
; (See Colin's comments for the Accumulate function.)

  if POOLSIZE
MixOutQueue     LABEL NEAR
        mov     cx, [Quality]
  if __FULLHASH
        cmp     bx, (512 shl FRACBITS) ; This is much more conservative
        jne     @AccNoOut
  else
        cmp     bx, cx
        jb      @AccNoOut
  endif
        shr     cx, 3+FRACBITS
        test    cx, cx
        jz      @AccNoOut
        mov     bx, [BytesAvail]
        push    es ds
        pop     es
        mov     di, OFFSET TransferPool
        cld
        call    Read
        pop     es
  if __FULLHASH
        mov     FreshCount, 0
  endif
        mov     si, [Quality]
        shr     si, 3+FRACBITS
        sub     si, 4
        mov     bx, [OutIndex]
        and     bx, (POOLSIZE-1)*4 ; (!) POOLSIZE must be divisible by 4!
@AccOutLoop:
        xor     eax, eax
        xchg    eax, TransferPool[si]
  if __NOHASHOUT
        rol     DWORD PTR OutPool[bx], 7
        xor     DWORD PTR OutPool[bx], eax
        add     bx, 4
        and     bx, (POOLSIZE-1)*4 ; (!) POOLSIZE must be divisible by 4!
  else
        add     bx, (OutTap6-OutTap5+1)*4
        and     bx, (OutTap6-1)*4
        xor     eax, DWORD PTR OutPool[bx]
        add     bx, (OutTap5-OutTap4)*4
        and     bx, (OutTap6-1)*4
        xor     eax, DWORD PTR OutPool[bx]
        add     bx, (OutTap4-OutTap3)*4
        and     bx, (OutTap6-1)*4
        xor     eax, DWORD PTR OutPool[bx]
        add     bx, (OutTap3-OutTap2)*4
        and     bx, (OutTap6-1)*4
        xor     eax, DWORD PTR OutPool[bx]
        add     bx, (OutTap2-OutTap1)*4
        and     bx, (OutTap6-1)*4
        xor     eax, DWORD PTR OutPool[bx]
        add     bx, OutTap1*4
        and     bx, (OutTap6-1)*4
        xor     eax, DWORD PTR OutPool[bx]
        rol     eax, 7
        mov     DWORD PTR OutPool[bx], eax
  endif
        cmp     [OutQueue], (POOLSIZE*4)
        je      SHORT @Acc01
        add     [OutQueue], 4
@Acc01: sub     si, 4
        jns     @AccOutLoop
        add     bx, 3
        and     bx, (POOLSIZE*4)-1
        mov     [OutIndex], bx
@AccNoOut:
  endif
@ceNoEntropy:
        mov     ax, bp          ; Restore ax

Accumulate      LABEL   NEAR
  ife NOMIX
; --------------------------------------------------------------------------
; This code written by Colin Plumb
;
; The accumulate function is designed to produce a large and
; high-quality, but not cryptographically secure hash of the input words
; very quickly based on the key scheduling technique used in the SHA.1
; secure hash algorithm, based on an irreducible word-wide polynomial
; (over the integers modulo 2, GF(2)) with a rotate thrown in mix the
; bits of each word amongst themselves.
;
; The pool is 512 bits, and the words are 16 bits wide, so there are
; 32 words in the pool, and thus we need a degree-32 polynomial.
;
; We've gone through several polynomials.  They all have the lowest
; number of non-zero terms possible which is consistent with having
; an irreducible polynomial of degree 32, namely 5.
; (If it were even, the polynomial would be divisible by (x+1), and
; since the degree is a multiple of 8, no trinomial is irreducible.)
;
; 0.3.4 used x^32 + x^25 + x^15 + x^2 + 1
;   This was changed to be primitive (as well as irreducible) and
;   have a lower third-lowest term, which causes faster mixing.
; 0.3.5 used x^32 + x^19 + x^6 + x^2 + 1
;   This was changed to have an *odd* third-lowest term,
;   since otherwise only the even words of the pool are affected
;   by stirring until 19 steps later.
; 0.3.6 and above use x^32 + x^17 + x^3 + x^2 + 1
;
; In pseudo-C code, for each new sampled word x, the mixing function is:
;   i = i+1 & 31;
;   x ^= pool[i] ^ pool[i-2 & 31] ^ pool[i-3 & 31] ^ pool[i-17 & 31]
;   rotate x left 7 bits
;   pool[i] = x

tap1	equ	2
tap2	equ	3
tap3	equ	17
tap4	equ	32	; Must equal pool size (and be a power of 2)
 
	mov	bx, index
	add	bx, (tap4-tap3)*2 + 2
	and	bx, (tap4-1)*2
	xor	ax, WORD PTR pool[bx]
	add	bx, (tap3-tap2)*2
	and	bx, (tap4-1)*2
	xor	ax, WORD PTR pool[bx]
	add	bx, (tap2-tap1)*2
	and	bx, (tap4-1)*2
	xor	ax, WORD PTR pool[bx]
	add	bx, tap1*2
	and	bx, (tap4-1)*2
	mov	index, bx
	xor	ax, WORD PTR pool[bx]
	rol	ax, 7
        mov     WORD PTR pool[bx], ax
  else
; ----- Unmixed accumulation simply adds the new samples to the queue.
;       This is done only to get a rough estimate of the raw samples
;       added by a particular method. (!) FOR DEBUGGING/TESTING ONLY.

        %out    **Warning: accumulate() will not mix the samples!
        mov     bx, Index
        add     bx, 2
        and     bx, 62
        mov     Index, bx
    if 1
        mov     WORD PTR pool[bx], ax
    else
        xor     WORD PTR pool[bx], ax
    endif
  endif
;        popf
        ret

  if POOLSIZE
TransferPool    LABEL   DWORD
        DD      16 dup (?)
  endif

EndOfDevice equ $ ; Everything after here will not be resident in memory
; --------------------------------------------------------------------------
; All code after this point is only needed for installation of the driver
; --------------------------------------------------------------------------
	include	initnoise.inc
_TEXT   ENDS
        END start



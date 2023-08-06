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

  if __CPU
; --------------------------------------------------------------------------
; Identify the CPU (code added in Version 0.3.5 by Colin Plumb)
;       Testing for 486 added Version 0.6.2 (untested on a 386)

; RDTSC	Caveat:	Allegedly some 486s support timestamp counter while some
;	586s may not.  If implementing RDTSC (0Fh 31h),	first try to set
;	bit 21 of EFLAGS. If it	sets, processor	supports CPUID instruction
;	(some latter 386 and 486s support this).  Set eax to 1,	then do	a
;	CPUID... bit 4 of edx should indicate if timestamp counter.

  if __CPU ge 4

;cpuid	 MACRO		 ; for older assmeblers	w/out .586 instructs
;	 DB 0Fh, A2h
;        ENDM
  endif

TestCPU         PROC    NEAR
; ----- The code here was (re)written for v0.3.6 by Colin Plumb
		push	sp
		pop	ax
		sub	ax, sp
                jne     SHORT @error

		mov	al, 4
		shl	al, 1	; Expected to produce ac = 0 on	a '286
		lahf
		shl	al, 1	; Expected to produce ac = 1 on	a '286
		mov	al, ah
		lahf
		xor	al, ah	; Xor the flags	together to detect the difference
		mov	ah, al	; Clear	ah if al is clear, leave Z flag	alone
                jnz     SHORT @error
@is386          LABEL   NEAR
  if __CPU ge 4                     ; test 486 (added v0.62)
                mov     bx, sp      ; save sp
                and     sp, not 3   ; allign dword
                pushfd              ; get EFLAGS
                pop     eax
                mov     ecx, eax    ; save
                btc     eax, 18     ; flip AC bit (#18) in EFLAGS
                push    eax
                popfd
                pushfd
                pop     eax         ; get EFLAGS
                mov     sp, bx      ; restore stack
                push    ecx         ; restore flags
                popfd
                xor     eax, ecx    ; was flag toggled?
                je      SHORT @error ; if unsettable, it's a 386
@is486          LABEL   NEAR
  if (__CPU ge 5) or (__UseTSC)
                bts     ecx, 21     ; try to set bit 21 of EFLAGS
                push    ecx         ; if it sets, we have the cpuid instruc.
                popfd               ;   otherwise it's a 486 and/or it does
                pushfd              ;   not have the rdtsc instruction
                pop     eax
                bt      eax, 21
                jnc     SHORT @error
                mov     eax, 1      ; do cpuid
                cpuid
                shr     ax, 8       ; processor id in bits 8-11
                and     al, 0Fh
                cmp     al, __CPU   ; check it; if below, an error
                jb      SHORT @error
    if __UseTSC                     ; check for rdtsc instruction
                bt      edx, 4
                jnc     SHORT @error
    endif
  endif
                xor     ax, ax
  endif
		ret

@error          LABEL   NEAR    ; Invalid CPU type
                Printf  <OFFSET CPUError>
		mov	ax, -1	; make sure ax != 0
                ret
TestCPU         ENDP

; --------------------------------------------------------------------------
CPUError LABEL  BYTE ; Hardcoded required CPU
        DB CR, LF
        DB (__CPU+30h), "86 or later processor "
  if __UseTSC
        DB "with TSC "
  endif
        DB "is required!", CR, LF, 0
  endif

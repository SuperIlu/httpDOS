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

; --------------------------------------------------------------------------
; NOISE.SYS v0.53+ API Routines and Data

; --------------------------------------------------------------------------
IOCTL_Read      PROC    NEAR                    ; Updated IOCTL
                mov     si, OFFSET ControlOut
                mov     cx, es:[di].ReqHdr.Siz  ; Bytes to read
                cmp     cx, szControlOut
                jna     SHORT @@irSizeOk
                mov     cx, szControlOut        ; Don't read more than that!
@@irSizeOk:
                mov     es:[di].ReqHdr.Siz, cx
                les     di, es:[di].ReqHdr.Addr ; Address to read to
                cld
                rep     movsb
                jmp     rdevok
IOCTL_Read      ENDP


; --------------------------------------------------------------------------
IdTag           equ     "noiz"  ; Id = 0x6e6f697a

ControlOut      LABEL   BYTE
                DW      szControlOut    ; Size of the output record
ReturnTag       DD      IdTag           ; Id tag
                DW      NoiseVersion    ; Version
HookAPI         DB      DefaultInt      ; Int used by API
HookFunc        DB      'n'             ; AH = HookFunc
FarCallAddr     DW      Offset FarCallHook
                DW      0
Flags           LABEL   WORD            ; Flags
                DD      DEFAULTFLAGS   
Quality         DW     QualityThreshold ; Output threshold
UsageFlag       DB      0               ; Multi-tasking friendliness
DosVersionMaj   DB      0               ; DOS Version (Major)
@StartTables    LABEL   WORD            ; -----
Counter         DD      0               ; Counter used for estimating entropy
FreshCount      DW      0               ; total fresh bits in pool
Outqueue        DW      0               ; Bytes waiting in outpool[]
szControlOut    equ     $-ControlOut

LastTick        LABEL WORD     ; Last sample added to entropy pool
                DW      0
  if N
CurrentHash     LABEL WORD  ; hash of last N samples (for table lookup)
                DW      0
lastNtable      LABEL WORD   ; The last N samples hashed with index
                DW      N dup (0)
indexNtable     LABEL WORD
                DW      (N-1)*2
  endif
deltatable      LABEL WORD   ; Table of when a hash last occurred
                DW      TABLESIZE dup (0)
szTables        equ     $-@StartTables

; --------------------------------------------------------------------------
@ReturnJump     LABEL   DWORD
                DD      ?
ResetNoiseHook  LABEL   FAR
; ResetNoiseHook() -  resets tables, etc. Trashes ax, cx, di, es, flags
; Returns: NC if Ok, CF if driver is currently in use.
  if 0 ; (!) Buggy: often the driver is in use, and can't be reset
                cmp     BYTE PTR [UsageFlag], 0
                je      @Resettable
                stc
                retf
@Resettable     LABEL   NEAR
                cld
                xor     ax, ax
                push    cs
                pop     es
                mov     di, OFFSET @StartTables
                mov     cx, szTables/2
                rep     stosw
                mov     [Partial], 8000h
                clc
  endif
                retf
                ibm_isp NoiseHook, ResetNoiseHook
@NoiseHookEntry LABEL   NEAR
                cmp     ah, cs:[HookFunc]  ; are we being called?
                je      SHORT @HelloSailor ; if yes, groovy
                jmp     DWORD PTR cs:[NoiseHook+2] ; otherwise continue
FarCallHook     LABEL   FAR                ; hook for far calls
                pushf
                jmp     SHORT @HelloSailor ; ignore AH (v0.6)
@ReturnOk:      dec     BYTE PTR [UsageFlag]
                clc                        ; CF=0 (Ok)
@ReturnMisc:    pop     bp si ds
                jmp     DWORD PTR cs:[@ReturnJump]
@AlreadyUsed:   mov     al, apiUseErr      ; API already in use
@ReturnErr:     dec     BYTE PTR [UsageFlag]
                stc                        ; CF=1 (Error)
                jmp     SHORT @ReturnMisc
;;                pop     bp si ds
;;                jmp     DWORD PTR cs:[@ReturnJump]
@HelloSailor:   pop     DWORD PTR cs:[@ReturnJump]
                popf
                push    ds si bp cs
                pop     ds
                inc     BYTE PTR [UsageFlag]
                cmp     al, NumFunctions
                jb      SHORT @FuncOk
@Invalid:       mov     al, apiInvalid     ; Bad function number
                jmp     SHORT @ReturnErr
@FuncOk:        mov     ah, 0
                add     ax, ax
                mov     si, ax
                jmp     @FuncTbl[si]
@FuncTbl        LABEL   WORD
                DW      @AMIS00       ; Installation Check (AL=0)
                DW      @AMIS01       ; Get Entryy Point (AL=1)
                DW      @AMISUnimplem ;
                DW      @AMISUnimplem ;
                DW      @AMIS04       ; Get Interrupt Hook List (AL=4)
                DW      @AMISUnimplem ;
                DW      @AMIS06       ; Reserved for future use
                DW      @Invalid      ;
                DW      @Invalid      ;
                DW      @Invalid      ;
                DW      @Invalid      ;
                DW      @Invalid      ;
                DW      @Invalid      ;
                DW      @Invalid      ;
                DW      @Invalid      ;
                DW      @Invalid      ;
                DW      @InstallCheck ; AL = 10h
                DW      @GetEstimate  ; AL = 11h
                DW      @SampleTimer, @AddSample
                DW      @GetFlags, @SetFlags
                DW      @ReadPRNG, @ReadRAND, @ReadControlTbl
NumFunctions    equ     ($ - @FuncTbl)/2

@AMIS00         LABEL   NEAR            ; Installation Check (AL=0)
                mov     al, apiOk
                mov     cx, NoiseVersion
                mov     dx, cs
                mov     di, OFFSET SignatureString
                jmp     SHORT @ReturnOk
SignatureString LABEL   BYTE
                DB      "WlkngOwl", "NoiseSYS", 0
@AMIS01         LABEL   NEAR            ; Get Entry Point (AL=1)
                mov     al, apiOk
                mov     dx, cs
                mov     bx, OFFSET FarCallHook ; or should this be DI reg?
                jmp     @ReturnOk
@HookList       LABEL   BYTE
        REPT 12 ; (!) Install_Int() routine does not check boundaries here!
                DB      DefaultInt
                DW      OFFSET NoiseHook
        ENDM
@AMIS04         LABEL   NEAR
                mov     al, 04h
                mov     dx, cs
                mov     bx, OFFSET @HookList
                jmp     @ReturnOk
@AMIS06         LABEL   NEAR    ; AMIS v3.5.9 proposed spec
                mov     dx, cs
                mov     bx, OFFSET rheader
                mov     ax, 2
                jmp     @ReturnOk
@AMISUnimplem   LABEL   NEAR
                mov     al, apiInvalid
                jmp     @ReturnErr
; --------------------------------------------------------------------------
@InstallCheck   LABEL   NEAR            ; Get Driver Status (AL=10h)
                mov     bh, [UsageFlag] ;          BH  = Usage Count
                mov     bl, (NumFunctions-1) ;     BL  = Highest function
  ife POOLSIZE
                mov     dx, 64          ;          DX  = Max bytes waiting
  else
                mov     dx, POOLSIZE*4
  endif
@GetWaiting     LABEL   NEAR            ;          CX  = No. bytes waiting
  ife POOLSIZE
                mov     cx, [FreshCount]
                shr     cx, 3+FRACBITS
  else
                mov     cx, [OutQueue]
  endif
                mov     ax, (__CPU shl 8)+apiOk ;  AH  = Build CPU, AL = Ok
                jmp     @ReturnOk
@GetEstimate    LABEL   NEAR            ; Get Estimated Entropy  (AH=11h)
                movzx   ebx, WORD PTR [OutQueue]
                shl     ebx, 3+FRACBITS
                mov     al, apiOk
  ife __FULLHASH
                movzx   edx, [FreshCount]
                add     ebx, edx        ;          EBX = Estimated bit count
  endif
                mov     cl, FRACBITS    ;          CL  = FRACBITS
                mov     edx, [Counter]  ;          EDX = No. of samples
                jmp     @ReturnOk
  if __AllowAddAPI
@SampleTimer    LABEL   NEAR            ; Sample timer (AL=12h)
                call    Sample
                jmp     SHORT @GetWaiting
@AddSample      LABEL   NEAR            ; Add sample (AL=13h, DX=sample)
                mov     ax, dx
                call    TrackDeltas
                jmp     SHORT @GetWaiting
  else
@SampleTimer    LABEL   NEAR            ; Sample timer (AL=12h)
@AddSample      LABEL   NEAR            ; Add sample (AL=13h)
  endif
@Disabled       LABEL   NEAR
                mov     al, apiDisabled
                jmp     @ReturnErr
@GetFlags       LABEL   NEAR            ; Get flags (AL=14h)
                mov     al, apiOk
                mov     bx, [Flags]     ; BX = flags
                mov     cx, SettableFlagsMask
                jmp     @ReturnOk
@SetFlags       LABEL   NEAR            ; Set flags (AL=15h)
                and     bx, SettableFlagsMask
                and     [Flags], not SettableFlagsMask
                or      [Flags], bx
                jmp     SHORT @GetFlags
@ReadPRNG       LABEL   NEAR            ; Read URANDOM bytes (AL=16h)
                push    cx
                cld
                mov     bx, [BytesAvail] ; CX=Length, ES:[DI]->buffer
                call    Read
                mov     al, apiOk
                pop     cx
                jmp     @ReturnOk
@ReadRAND       LABEL   NEAR            ; Read RANDOM bytes
                mov     al, apiUseErr
                cmp     [UsageFlag], 1
                ja      SHORT @rrReadNothing
  if POOLSIZE                           ; Read bytes from output pool
                mov     ax, [OutQueue]  ; check if pool has any bytes
                test    ax, ax
                jnz     SHORT @rrAboveThresh0
                mov     al, apiEmpty
@rrReadNothing:
                xor     cx, cx
                jmp     @ReturnErr
@rrAboveThresh0:
  else                                  ; Read bytes from hash
                mov     ax, [FreshCount] ; check if pool has entropy
                cmp     ax, EntropyThreshold ; (*) Change to mem ref?
                ja      SHORT @rrAboveThresh1
                mov     al, apiUseErr
@rrReadNothing:
                xor     cx, cx
                jmp     @ReturnErr
@rrAboveThresh1:
                shr     ax, 3+FRACBITS         ; bytes requested > entropy?
  endif
                cmp     cx, ax
                jna     SHORT @rrEnough
                mov     cx, ax
@rrEnough:
  if POOLSIZE
                sub     [OutQueue], cx
                mov     bx, [OutIndex]
                add     bx, 3
                and     bx, (POOLSIZE*4)-1
                push    cx
                cld
@rrPoolCopyLoop:
                mov     al, bl
                xchg    al, BYTE PTR OutPool[bx]
                dec     bx
@rrr00:         and     bx, (POOLSIZE*4)-1
                stosb
                loop    @rrPoolCopyLoop
                pop     cx
                mov     [OutIndex], bx
  else
                jmp     SHORT @ReadPRNG
  endif
@rrExit:        mov     al, apiOk
                jmp     @ReturnOk
@ReadControlTbl LABEL   NEAR
  if 1
                jmp     @Disabled
  else
                mov     si, OFFSET ControlOut
                cmp     cx, szControlOut
                jna     SHORT @@rctSizeOk
                mov     cx, szControlOut        ; Don't read more than that!
@@rctSizeOk:
                mov     ax, cx
                cld
                rep     movsb
                mov     cx, ax
                jmp     SHORT @rrExit
   endif

; --------------------------------------------------------------------------
; A hook for Interrupt 15h (for future development)

 if __Sample15
                ibm_isp _Multitask, Reset_MultiplexServ
  if __SamplDrift
                cmp     ah, 83h
                je      @mt15VoidTimer
                cmp     ah, 86h
                je      @mt15VoidTimer
                jmp     SHORT @mt15Exit
@mt15VoidTimer  LABEL   NEAR
                test    BYTE PTR @SpinFlag, 81h
                jnz     SHORT @mt15Exit
                pop     ebx
                popf
                push    ebx
                stc
                retf
  endif
@mt15Exit       LABEL   NEAR
                jmp     DWORD PTR cs:[_Multitask+2]
 endif

; --------------------------------------------------------------------------
; Hook for Interrupt 2Fh (Multiplex Services)

;
; TSR Info Structure for Windows-aware TSR's and device drivers
;
TSR_Info_Struc  STRUC
   TSR_Next           dd    ?     ; put ES:DI here and point ES:DI to struc
   TSR_PSP_Segment    dw    ?     ; put CS here
   TSR_API_Ver_ID     dw    100h  ; Set to 100h
   TSR_Exec_Flags     dw    0
   TSR_Exec_Cmd_Show  dw    0
   TSR_Exec_Cmd       dd    0
   TSR_Reserved       db    4 dup (0)
   TSR_ID_Block       dd    0
   TSR_Data_Block     dd    0
TSR_Info_Struc ENDS

NoiseWin        TSR_Info_Struc <,,100h,,,,,,>
NoiseID         LABEL FAR
                DW  IdLen
                DB  "NOISE.SYS", 0
IDLen           equ ($-NoiseID)-2

                ibm_isp _MultiplexServ
; Normally we'd call the original handler first, but in this case it
; interferes with values passed to and from orig. handler, so we check
; values passed first then jump.
                pushf

; <---- Add sampling of network (and CD-ROM?) calls to function 11h?

  if __samplcdrom
; Note: This does not work with SONY ATAPI_CD.SYS driver.
;       (Since I am using a SONY CD-ROM, I am unable to test this code)

                cmp     ah, 15h                 ; CD-ROM access (15h)?
                jne     SHORT @NoCD
                pushad                          ; save registers
                push    ds
                call    Sample
                pop     ds                      ; restore regs
                popad
                jmp     SHORT @QuitMServ
@NoCD           LABEL   NEAR
  endif
                cmp     ah, 16h                 ; Win broadcasts?
                jne     SHORT @QuitMServ
  if 1
                cmp     al, 0Bh                 ; TSR ID?
                jne     SHORT @wc00
                mov     Word Ptr [NoiseWin], di
                mov     Word Ptr [NoiseWin+2], es
                mov     di, cs
                mov     es, di
                mov     di, Offset NoiseWin
                jmp     SHORT @QuitMServ
@wc00:
  endif
;
@ToggleWinFLag  LABEL   NEAR
                cmp     al, 8                   ; Check for init/close msgs
                jb      SHORT @QuitMServ
                cmp     al, 9
                ja      SHORT @QuitMServ
                pushad
                push    ds
                xor     BYTE PTR [Flags], WindowsFlag ; toggle flag
                jz      SHORT @NotInWin
;;  ifdef WIN95
;;  endif
@NotInWin       LABEL   NEAR
  if __SamplDrift
                mov     @SpinFlag, 80h
  endif
                call    Sample ; Sample enter/exit times for Windows...
                pop     ds
                popad
@QuitMServ      LABEL   NEAR
                popf
                jmp     DWORD PTR cs:[_MultiplexServ+2]

; --------------------------------------------------------------------------
; A hook for Interrupt 21h (DOS)

  if (__SamplExec) or (__SamplFlush)
;@LastExec       LABEL   WORD
;                DW      ?
  endif
  if (__DosSpinner) and (__Sample21 eq 0)
@DosSpinner     LABEL   WORD
                DW      0
@LastDosSpin    LABEL   WORD
                DW      0
  endif
                ibm_isp _NewInt21, Reset_MultiplexServ
  if (__DosSpinner) and (__Sample21 eq 0)
                inc     WORD PTR cs:@DosSpinner
  ; ----- Count the number of calls to DOS between every clock tick
  endif
  if (__SamplExec) and (__Sample21 eq 0)
                cmp     ah, 0             ; Old form of exit (still used)
                je      SHORT @SampleExec
                cmp     ah, 4Bh           ; Exec/Load overlay
                je      SHORT @SampleExec
                cmp     ah, 4Ch           ; Terminate process
                je      SHORT @SampleExec
  endif
  if (__SamplFlush) and (__Sample21 eq 0)
                cmp     ah, 68h           ; Commit (flush) file
                je      SHORT @SampleExec
  endif
  @OrigDOS:
  ife (__Sample21)
                jmp     DWORD PTR cs:[_NewInt21+2]
  endif
  if (__SamplExec) or (__SamplFlush) or (__Sample21)
  @SampleExec   LABEL   NEAR
;; What this needs to do is set the InDOS flag?
;                pushf
;                cli
                pushad
                push    ds
                call    Sample
                pop     ds
                popad
;                popf
  endif
                jmp     DWORD PTR cs:[_NewInt21+2]


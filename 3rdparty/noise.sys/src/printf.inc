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
; A rudimentary	printf()-like function by Robert Rothenburg Walking-Owl
; \-codes are not supported, so	inline them instead (use "",13,10 not "\n")
; "%%" to print	'%', "%c" to print a character argument, "%s" for string
; "%d" to print	unsigned int, "%x" to print unsigned int as hexidecimal
; %d and %x can	be in form of %#d or %#x, where	0 <= # <= 9 to right-justify
; Strings are assumed to be null-terminated. Arguments are passed on stack.
; printf("My name is: %s",name)	requires push name, then "My name is: %s"

stdout	equ 1
stderr	equ 2

MPrintf         MACRO   parms:VARARG
		push	parms
		call	_printf
		ENDM

Printf		MACRO	parms:VARARG
                LOCAL   @00
                test    [UserOptionFlags], SuppressTechie
                jnz     @00
		push	parms
		call	_printf
@00:
		ENDM

TestQuiet	MACRO
		LOCAL	@@0
;;                test    [Flags], QuietFlag
;;                jz      SHORT @@0
;;                ret
@@0:
		ENDM

stdhandle	LABEL	WORD   ; this allows an	API to output msgs to a
		DW	stderr ;  handle

_printf		PROC	NEAR ; control string offset pushed on stack
		pop	bp
		pop	si
		cld
@mploop:
		lodsb
		test	al, al
		jz	SHORT @mpexit
		cmp	al, '%'
		je	SHORT @mpescape
@mpputc:
		call	_putc
		jmp	SHORT @mploop
@mpexit:
		push	bp
		ret

@mpescape:
		mov	BYTE PTR padding, 0
@mpescode:
		lodsb
		test	al, al
		jz	SHORT @mpexit
		cmp	al, '%'			; %% = '%' character
		je	SHORT @mpputc		; %#d =	formmating codes
		cmp	al, '0'			; implemented only for %d
		jb	SHORT @mpnfmt		; 0-9 spaces padding only
		cmp	al, '9'
		ja	SHORT @mpnfmt
		sub	al, '0'
		mov	BYTE PTR padding, al
		jmp	SHORT @mpescode
@mpnfmt:
		cmp	al, 'c'			; %c = character
		jne	SHORT @mpnchar
		pop	ax
		jmp	SHORT @mpputc
@mpnchar:
		cmp	al, 's'			; %s = string
		jne	SHORT @mpnstr
		pop	dx
		call	_putstr
		jmp	SHORT @mploop
@mpnstr:
		cmp	al, 'd'			; %d = unsigned	int
		jne	SHORT @mpnint
		pop	ax
		mov	cx, 10
		call	_putd
		jmp	SHORT @mploop
@mpnint:
		cmp	al, 'x'			; %x = unsigned	int (hex)
		jne	SHORT @mpnhex
		pop	ax
		mov	cx, 16
		call	_putd
		jmp	SHORT @mploop
@mpnhex:
		mov	al, '?'			; unknown escape code
		jmp	@mpputc
_printf		ENDP

_strlen		PROC	NEAR ; call ds:[si] -> string; ret cx == length
		mov	cx, -1
@eosloop:	inc	cx
		lodsb
		test	al, al
		jnz	SHORT @eosloop
		ret
_strlen		ENDP

_putstr		PROC	NEAR ; call ds:[dx] -> string
		TestQuiet
		push	si
		mov	si, dx
		call	_strlen
                mov     bx, [stdhandle]
		mov	ah, 40h
		int	DOS
		pop	si
		ret
_putstr		ENDP

_putc		PROC	NEAR
		TestQuiet
                mov     dx, [stdhandle]
		mov	bx, OFFSET putcbuff
		mov	[bx], al
		mov	cx, 1
		xchg	bx, dx
		mov	ah, 40h
		int	DOS
		ret
_putc		ENDP

_putd		PROC	NEAR	; ax = number, cx = base
		mov	bx, OFFSET scratchbuff
@pdloop:
		xor	dx, dx
		div	cx
		add	dl, '0'
		cmp	dl, '9'
		jna	SHORT @pddec
		add	dl, 7
@pddec:
		dec	bx
		mov	[bx], dl
		test	ax, ax
		jnz	SHORT @pdloop
@pddone:	cmp	BYTE PTR padding, 0
		je	SHORT @pdnopad
		mov	cx, OFFSET scratchbuff
		sub	cx, bx
		cmp	cl, padding
		jnb	SHORT @pdnopad
		neg	cl
                add     cl, [padding]
@pdpadloop:	dec	bx
		mov	BYTE PTR [bx], '0'   ; maybe allow pad definition?
;		 cmp	 bx, OFFSET putcbuff
;		 je	 SHORT @pdnopad
		loop	@pdpadloop
@pdnopad:
		mov	dx, bx
		jmp	_putstr
_putd		ENDP

putcbuff	LABEL	BYTE
		DB	15 dup (?)
scratchbuff	LABEL	BYTE
		DB	0
padding		LABEL	BYTE
		DB	0


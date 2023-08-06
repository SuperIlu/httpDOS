COMMENT	|
============================================================================

  Secure Hash Transformation Code. (coded for NOISE.SYS v0.3
  /dev/random driver) by Colin Plumb, December 1995.  Minor changes
  made by Robert Rothenburg Walking-Owl for use with the driver,
  January 1996.

  Optimized shifting based on "fast shift" bug in 486 (rol reg,imm8+32 is
  faster than rol reg,1 by one cycle), May 1996.

  License changed to BSD, September 2004.

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

; registers: a,	b, c, d, e are edi, ebx, ecx, edx, ebp.	 eax is	temp.
; The basic sub-round is:
; a' = e + ROTL(5,a) + f(b, c, d) + k +	data;
; b' = a;
; c' = ROTL(30,b);
; d' = c;
; e' = d;
; Where	f(b, c,	d) is one of:
; f1(x,y,z) = (x & y) |	(~x & z)
;	    = z	^ (x & (y ^ z))
; f2(x,y,z) = x	^ y ^ z
; f3(x,y,z) = (x & y) |	(x & z)	| (y & z)
;	    = (x & y) |	(z & (x	| y))
;	    = (x & y) |	(z & (x	^ y))
;	    = (x & y) +	(z & (x	^ y))
; f4(x,y,z) = x	^ y ^ z

__revised       equ     1               ; 
SHA_VERSION     equ     __revised       ; FIPS 180-1

SHATransform:
	push	ebp
	push	edi
	push	si

	mov	si, OFFSET pool	+ (4*16)
	mov	cx, 64
expandloop:
	mov	eax, [si-(4*3)]
	xor	eax, [si-(4*8)]
	xor	eax, [si-(4*14)]
	xor	eax, [si-(4*16)]
if SHA_VERSION
  if __CPU eq 4
        rol     eax, 33 ; rol reg,imm8+32 is faster on 486 than rol reg,1
  else
        rol     eax, 1
  endif
endif
	mov	[si], eax
	add	si, 4
	loop	expandloop

	mov	edi, state
	mov	ebx, state+4
	mov	ecx, state+8
	mov	edx, state+12
	mov	ebp, state+16

	mov	si, OFFSET	pool
round1:
	mov	 eax, edx
	xor	 eax, ecx
	and	 eax, ebx
	xor	 eax, edx
	add	 ebp, eax

	mov	eax, edi
	rol	eax, 5
	add	eax, [si]
	add	si, 4
	add	eax, 5A827999h
	add	eax, ebp

	mov	ebp, edx
	mov	edx, ecx
	mov	ecx, ebx
	ror	ecx, 2
	mov	ebx, edi
	mov	edi, eax

	cmp	si, OFFSET pool	+ (4*20)
	jb	short round1
round2:
	mov	 eax, edx
	xor	 eax, ecx
	xor	 eax, ebx
	add	 ebp, eax

	mov	eax, edi
	rol	eax, 5
	add	eax, [si]
	add	si, 4
	add	eax, 6ED9EBA1h
	add	eax, ebp

	mov	ebp, edx
	mov	edx, ecx
	mov	ecx, ebx
	ror	ecx, 2
	mov	ebx, edi
	mov	edi, eax

	cmp	si, OFFSET pool	+ (4*40)
	jb	short round2
round3:
	mov	 eax, ebx
	and	 eax, ecx
	add	 ebp, eax
	mov	 eax, ebx
	xor	 eax, ecx
	and	 eax, edx
	add	 ebp, eax

	mov	eax, edi
	rol	eax, 5
	add	eax, [si]
	add	si, 4
	add	eax, 8F1BBCDCh
	add	eax, ebp

	mov	ebp, edx
	mov	edx, ecx
	mov	ecx, ebx
	ror	ecx, 2
	mov	ebx, edi
	mov	edi, eax

	cmp	si, OFFSET pool	+ (4*60)
	jb	short round3
round4:
	mov	 eax, edx
	xor	 eax, ecx
	xor	 eax, ebx
	add	 ebp, eax

	mov	eax, edi
	rol	eax, 5
	add	eax, [si]
	add	si, 4
	add	eax, 0CA62C1D6h
	add	eax, ebp

	mov	ebp, edx
	mov	edx, ecx
	mov	ecx, ebx
	ror	ecx, 2
	mov	ebx, edi
	mov	edi, eax

	cmp	si, OFFSET pool	+ (4*80)
	jb	short round4

	add	state   , edi
	add	state+ 4, ebx
	add	state+ 8, ecx
	add	state+12, edx
	add	state+16, ebp

  if 1
; Now to obliterate the	expanded bytes in the pool
	mov	bx, es	; save es and set es = ds
	mov	ax, ds
	mov	es, ax
	mov	di, OFFSET pool	+ (4*16)
	xor	eax, eax
	mov	cx, 64
	rep stosd
	mov	es, bx
  endif
	pop	si
	pop	edi
	pop	ebp
	ret

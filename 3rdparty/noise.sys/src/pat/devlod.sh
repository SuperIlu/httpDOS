#       This is a shell archive of patches for Jim Kyle's DEVLOD.
#       These patches enable DEVLOD to load drivers with multiple
#         devices, as well as work under DOS 6.
#
#       DEVLOD can be gotten via anonymous ftp from these sites:
#         ftp://garbo.uwasa.fi/pc/sysfile/devlod.zip
#         ftp://oak.oakland.edu/SimTel/msdos/sysfile/devlod.zip
#
#       To use this archive, you need utilities to extract shell
#         archives and apply patches made with diff.
#
#	Remove everything above and including the cut line.
#       Then run the rest of the file through sh.
#----cut here-----cut here-----cut here-----cut here----#
#!/bin/sh
# shar:    Shell Archiver
#	Run the following text with /bin/sh to create:
#	devlod.dif
#	movup.dif
#	c0.dif
#	makefile.dif
# This archive created: Sun Mar 31 00:46:16 1996
cat << \SHAR_EOF > devlod.dif
4a5,10
>  *     Further revisions added by Rob Rothenburg Walking-Owl - 1/96 *
>  *            Should work with DOS 6                                *
>  *            Some support for multiple drivers (in same file)      *
>  *            Fixed bug in executing drivers - 3/96                 *
>  *     Needs: Check if Windows is running                           *
>  *            Reject OS/2 device drivers (OS/2 flag in attrib word) *
8c14,15
<      
---
> 
> 
76a84,85
> void chainptr( void far *src, void far *dst ); /* in MOVUP.ASM file  */
> 
99c108
< void Put_Msg ( char *msg )
---
> void Put_Msg ( char *msg )  /* replaces printf()                    */
181a191
>     case  6:
210c220
<   copyptr ( &nxtdrvr, drvptr );         /* and old after new    */
---
>   chainptr ( &nxtdrvr, drvptr );         /* and old after new    */
217d226
<   /* This incorrectly bashes SUBSTed and network drives */
233c242
< { unsigned tmp;
---
> { unsigned tmp, next;
235,242d243
<   CmdPkt.command = INIT;        /* build command packet         */
<   CmdPkt.hdrlen = sizeof (struct packet);
<   CmdPkt.unit = 0;
<   CmdPkt.inpofs  = (unsigned)dvrarg;    /* points into cmd line */
<   CmdPkt.inpseg  = _psp;
<   /* can't really check for next drive here, because don't yet know
<      if this is a block driver or not */
<   CmdPkt.NextDrv = Next_Drive();
243a245,267
>   do {
>     CmdPkt.command = INIT;        /* build command packet         */
>     CmdPkt.hdrlen = sizeof (struct packet);
>     CmdPkt.unit = 0;
>     CmdPkt.inpofs  = (unsigned)dvrarg;    /* points into cmd line */
>     CmdPkt.inpseg  = _psp;
>     /* can't really check for next drive here, because don't yet know
>        if this is a block driver or not */
>     CmdPkt.NextDrv = Next_Drive();
> 
>     tmp = *((unsigned far *)drvptr+3);    /* STRATEGY pointer     */
>     driver = MK_FP( FP_SEG( drvptr ), tmp );
>     _ES = FP_SEG( (void far *)&CmdPkt );
>     _BX = FP_OFF( (void far *)&CmdPkt );
>     (*driver)();                  /* set up the packet address    */
> 
>     tmp = *((unsigned far *)drvptr+4);    /* COMMAND pointer      */
>     driver = MK_FP( FP_SEG( drvptr ), tmp );
>     (*driver)();                  /* do the initialization        */
> 
>     next = *((unsigned far *)drvptr);
>     drvptr = MK_FP( FP_SEG( drvptr ), next );
>   } while ((next!=0xffff) && (! ( CmdPkt.status & 0x8000 )));
245,254c269
<   tmp = *((unsigned far *)drvptr+3);    /* STRATEGY pointer     */
<   driver = MK_FP( FP_SEG( drvptr ), tmp );
<   _ES = FP_SEG( (void far *)&CmdPkt );
<   _BX = FP_OFF( (void far *)&CmdPkt );
<   (*driver)();                  /* set up the packet address    */
< 
<   tmp = *((unsigned far *)drvptr+4);    /* COMMAND pointer      */
<   driver = MK_FP( FP_SEG( drvptr ), tmp );
<   (*driver)();                  /* do the initialization        */
<   
---
>   drvptr  = MK_FP( _psp+0x10, 0 );      /* new driver's address */
350c365,372
<     Err_Halt ( "Device driver name required.");
---
>     Err_Halt ( "Loads device drivers from the command line.\n\n\r" \
>              "Usage: devlod device [device options]\n\r" \
>              "Example(s): devlod ansi.sys\n\r" \
>              "            devlod mtmcde.sys /d:mscd001\n\n\r" \
>              "Copyright (C)1990 by Jim Kyle - All Rights Reserved.\n\r" \
>              "Modified to handle drivers with multiple devices and to run\n\r" \
>              "under DOS 6 by Robert Rothenburg Walking-Owl, Jan-Mar 1996.\n\r" );
>                
SHAR_EOF
cat << \SHAR_EOF > movup.dif
7c7,11
< _TEXT   SEGMENT BYTE PUBLIC 'CODE'
---
> ; Minor optimizations made by Rob Rothenburg Walking-Owl
> 
>         .386
> 
> _TEXT   SEGMENT BYTE PUBLIC 'CODE' USE16
10c14
< _DATA   SEGMENT WORD PUBLIC 'DATA'
---
> _DATA   SEGMENT WORD PUBLIC 'DATA' USE16
13c17
< _BSS    SEGMENT WORD PUBLIC 'BSS'
---
> _BSS    SEGMENT WORD PUBLIC 'BSS' USE16
67,68c71
<         movsw
<         movsw
---
>         movsd
72,73c75
<         mov     sp, bp
<         pop     bp
---
>         pop     bp      ; unnecessary mov sp, bp removed
77c79,80
< _TEXT   ENDS
---
> ; ----- Chain seraching code added by Rob Rothenburg Walking-Owl, 1/14/96
>         PUBLIC  _chainptr
79c82,107
<         end
---
> _chainptr       PROC      NEAR
>         push    bp
>         mov     bp, sp
>         push    si
>         push    di
>         push    ds
>         lds     si,[bp+4]               ; source
>         les     di,[bp+8]               ; destination
>         mov     bx, di
> @2@0:
>         cmp     WORD PTR es:[di], -1    ; is it end of chain?
>         je      SHORT @2@1
>         add     WORD PTR es:[di], bx
>         mov     WORD PTR es:[di+2], es  ; set segment
>         mov     di, WORD PTR es:[di]    ; go to next device in chain
>         jmp     SHORT @2@0
> @2@1:
>         cld
>         movsd
>         pop     ds
>         pop     di
>         pop     si
>         pop     bp
>         ret
> _chainptr       ENDP
> 
80a109
> _TEXT   ENDS
81a111
>         end
SHAR_EOF
cat << \SHAR_EOF > c0.dif
1,4c1
<         NAME c0
< ;[]------------------------------------------------------------[] 
< ;|  C0.ASM -- Start Up Code                                     | 
< ;| based on Turbo-C startup code, extensively modified          |
---
>         NAME    c0
5a3,5
> ;|      C0.ASM -- Start Up Code                                 |
> ;|        based on Turbo-C startup code, extensively modified   |
> ;[]------------------------------------------------------------[]
7c7,9
< _TEXT   SEGMENT BYTE PUBLIC 'CODE'
---
>         .386 ; minor optimizations by Rob Rothenburg Walking-Owl, 3/96
> 
> _TEXT   SEGMENT BYTE PUBLIC 'CODE' USE16
10c12
< _DATA   SEGMENT WORD PUBLIC 'DATA'
---
> _DATA   SEGMENT WORD PUBLIC 'DATA' USE16
13c15
< _BSS    SEGMENT WORD PUBLIC 'BSS'
---
> _BSS    SEGMENT WORD PUBLIC 'BSS' USE16
64c66
<         jae     AskedStackOK    ; yes, use it
---
>         jae     SHORT AskedStackOK ; yes, use it
69c71
<         jb      InitFailed      ; DATA segment can NOT be > 64 Kbytes
---
>         jb      SHORT InitFailed ; DATA segment can NOT be > 64 Kbytes
71,73c73,75
<         jb      InitFailed      ; DATA segment can NOT be > 64 Kbytes
<         mov     cl, 4
<         shr     di, cl          ; $$$ Do not destroy CL $$$
---
>         jb      SHORT InitFailed ; DATA segment can NOT be > 64 Kbytes
> ;;      mov     cl, 4
>         shr     di, 4           ; $$$ Do not destroy CL $$$
76c78
<         jnb     TooMuchRAM      ; Enough to run the program
---
>         jnb     SHORT TooMuchRAM ; Enough to run the program
87c89
<         shl     di, cl          ; $$$ CX is still equal to 4 $$$
---
>         shl     di, 4           ; $$$ CX is still equal to 4 $$$
101c103
<         shr     bx,cl           ; length in paragraphs
---
>         shr     bx, 4           ; length in paragraphs
112c114,118
<         rep     stosb
---
>         shr     cx, 1
>         jnc     SHORT @@0@1
>         stosb
> @@0@1:
>         rep     stosw
223,224d228
< 
< 
SHAR_EOF
cat << \SHAR_EOF > makefile.dif
3a4,5
> # Updated for BCC4 by Rob Walking-Owl 3/96
> 
11c13
<         tcc -c -ms devlod
---
>         bcc -3 -O2 -Z -c -mt devlod
17a20,21
> 
> 
SHAR_EOF
#	End of shell archive
exit 0

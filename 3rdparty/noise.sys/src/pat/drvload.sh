#       This is a shell archive of patches for Rick Knoblaugh's
#       DRVLOAD 1.0, from PC Magazine vol. 12 no. 19.  These
#       patches enable DRVLOAD to load drivers with multiple
#         devices.
#
#       DRVLOAD can be gotten via anonymous ftp from these sites:
#         ftp://garbo.uwasa.fi/pc/pcmagvol/vol12n19.zip
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
#	drvmain.dif
#	drvdata.dif
# This archive created: Wed Jan 17 01:56:45 1996
cat << \SHAR_EOF > drvmain.dif
*** drvmain.asm	Mon Oct 11 19:54:40 1993
--- drvmain1.asm	Wed Jan 17 01:38:50 1996
***************
*** 10,16 ****
  ; 7/04/93                      Rick Knoblaugh                  |
  ;--------------------------------------------------------------|
  ;include files                                                 |
! ;---------------------------------------------------------------    
                  include drvequ.inc
                  include drvstruc.inc
  
--- 10,18 ----
  ; 7/04/93                      Rick Knoblaugh                  |
  ;--------------------------------------------------------------|
  ;include files                                                 |
! ;---------------------------------------------------------------
! ;Changed by Robert Rothenburg Walking-Owl, Jan 96
! 
                  include drvequ.inc
                  include drvstruc.inc
  
***************
*** 422,427 ****
--- 424,431 ----
                  mov     ds, dx                  ;ds=driver header
                  mov     es, ax                  ;es=local data
  
+ init_loop:
+ 
  ;es:bx point to driver cmd request block
                  push    cs                      ;far
                  mov     ax, offset do_drv_i100  ;return address
***************
*** 446,452 ****
  ;One poorly designed driver we tested relied on ah being equal
  ;to zero.  Set it to zero just to make drivers of that nature
  ;happy.
! ;                
                  xor     ax, ax
                  retf                            ;make int call
  do_drv_i200:
--- 450,460 ----
  ;One poorly designed driver we tested relied on ah being equal
  ;to zero.  Set it to zero just to make drivers of that nature
  ;happy.
! 
! ; ...but several poorly designed utilities I've tested (this one
! ; is no exeption) did not account for multiple drivers in the
! ; same file....  --Rob Walking-Owl
! ;
                  xor     ax, ax
                  retf                            ;make int call
  do_drv_i200:
***************
*** 453,458 ****
--- 461,472 ----
  
                  test    es:[bx].rh_status, (mask err_bit) ;error?
                  jnz     do_drv_i900
+                 cmp     WORD PTR [si], -1
+                 je      SHORT init_done
+                 add     si, [si]
+                 jmp     SHORT init_loop
+ 
+ init_done:
                  mov     dx, ds                  ;start of driver
                  mov     ax, es:[bx].init_brk_seg
                  sub     ax, dx                  ;difference if any
***************
*** 803,814 ****
                  push    ds
                  lds     si, nul_dev_ptr         ;point to NUL header
                  mov     ax, [si].dev_chain.d_offset ;ptr to next drvr
!                 mov     dx, [si].dev_chain.d_segment
!                 
!                 cli          
                  mov     [si].dev_chain.d_offset, bx  ;put ours in list
                  mov     [si].dev_chain.d_segment, es
!                 mov     es:[bx].dev_chain.d_offset, ax  ;link to 
                  mov     es:[bx].dev_chain.d_segment, dx ;old 1st drvr
                  sti
                  pop     ds                
--- 817,836 ----
                  push    ds
                  lds     si, nul_dev_ptr         ;point to NUL header
                  mov     ax, [si].dev_chain.d_offset ;ptr to next drvr
!                 mov     dx, [si].dev_chain.d_segment                
! 
!                 cli
                  mov     [si].dev_chain.d_offset, bx  ;put ours in list
                  mov     [si].dev_chain.d_segment, es
! chain_loop:                                     ; look for end of list
!                 cmp     WORD PTR es:[bx].dev_chain.d_offset, -1
!                 je      SHORT end_of_list
!                 add     es:[bx].dev_chain.d_offset, bx  ; adjust pointer
!                 mov     es:[bx].dev_chain.d_segment, es
!                 mov     bx, es:[bx].dev_chain.d_offset  ; check next device
!                 jmp     SHORT chain_loop
! end_of_list:
!                 mov     es:[bx].dev_chain.d_offset, ax  ;link to
                  mov     es:[bx].dev_chain.d_segment, dx ;old 1st drvr
                  sti
                  pop     ds                
SHAR_EOF
cat << \SHAR_EOF > drvdata.dif
71c71
< sign_on_msg     db      CR, LF,"DRVLOAD Version 1.0  Copyright (c) 1993", CR, LF, \
---
> sign_on_msg     db      CR, LF,"DRVLOAD Version 1.0p  Copyright (c) 1993", CR, LF, \
SHAR_EOF
#	End of shell archive
exit 0

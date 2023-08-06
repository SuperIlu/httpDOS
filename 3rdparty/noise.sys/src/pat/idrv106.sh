#       This is a shell archive of patches for Janusz Wojcik's Idrv
#       These patches enable Idrv 1.06 to load drivers with multiple
#         devices, as well as work under DOS 6.
#
#       Idrv can be gotten via anonymous ftp from these sites:
#         ftp://oak.oakland.edu/SimTel/msdos/sysfile/idrv01.zip
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
#	idrv.dif
# This archive created: Mon Jan 15 23:58:26 1996
cat << \SHAR_EOF > idrv.dif
*** idrv.old	Mon Nov 25 14:51:04 1991
--- idrv.asm	Mon Jan 15 23:55:50 1996
***************
*** 9,14 ****
--- 9,17 ----
  ;						Lublin,Poland.		       ;
  ;..............................................................................;
  
+ ; Changes made by Robert Rothenburg Walking-Owl, January 1996
+ ;       Multiple-device drivers now supported
+ ;       Works under DOS 6
  
  		ASSUME DS:st_seg, SS:st_seg ,CS:st_seg ,ES:st_seg
  st_seg		SEGMENT	
***************
*** 298,304 ****
  ;	Device Headers of the rest  drivers				.......					.......
  ;..............................................................................
  
- 
  link_dev:
  		mov	es,dos_seg
  		mov	si,48h
--- 301,306 ----
***************
*** 308,316 ****
--- 310,328 ----
  		mov	ax,dev_seg
  		push	ds
  		mov	ds,ax
+ follow_chain:
+                 cmp     next_offs[bx], -1  ; check if device is last on chain
+                 je      end_of_chain       ; if so, update link
+                 add     next_offs[bx], bx  ; adjust offset
+                 mov     next_seg[bx], ax   ; set segment
+                 mov     bx, next_offs[bx]  ; go to next device on the chain
+                 jmp     short follow_chain                                
+ 
+ end_of_chain:
  		mov	next_offs[bx],si
  		mov	next_seg[bx],es
  		mov	bx,dev_attr[bx]		;get dev.attribute for later
+ 
  		pop	ds
  
  		pop	es
***************
*** 468,492 ****
  ;...............................................................................
  init_dev:
  		mov	si,drv_beg
! 		mov	ax,dev_seg
! 		mov	es,ax
  		mov	ax,es:dev_strategy[si]	;get addr. of strategy routine
  		mov	dev_offs,ax
  		mov	ax,es:dev_interrupt[si] ;get addr. of interrupt routine
  		push	ax			;save it for later
  
  		push	cs
  		pop	es			;ES = CS
  		mov	bx,offset Request_Header  ;ES:BX - Request Header
! 
  		call	dword ptr dev_offs	;Call dev. strategy routine
  		
  		pop	ax
! 		mov	dev_offs,ax
  
  		call	dword ptr dev_offs	;Call dev. interrupt routine
  		mov	ax,ending_offset
! 		or	ax,ax
  		jnz	quit_init
  		mov	ax,ending_segment
  		cmp	ax,dev_seg
--- 480,514 ----
  ;...............................................................................
  init_dev:
  		mov	si,drv_beg
! 
! init_loop:
!                 mov     es,dev_seg
  		mov	ax,es:dev_strategy[si]	;get addr. of strategy routine
  		mov	dev_offs,ax
  		mov	ax,es:dev_interrupt[si] ;get addr. of interrupt routine
+                 push    es                
  		push	ax			;save it for later
  
  		push	cs
  		pop	es			;ES = CS
  		mov	bx,offset Request_Header  ;ES:BX - Request Header
!                 
  		call	dword ptr dev_offs	;Call dev. strategy routine
  		
  		pop	ax
!                 mov     dev_offs, ax
  
  		call	dword ptr dev_offs	;Call dev. interrupt routine
+ 
+                 pop     es
+                 cmp     word ptr es:next_offs[si], -1 ; check ptr to next dvr
+                 je      end_init_chain
+                 mov     si, es:next_offs[si] ; go to next driver in chain
+                 jmp     short init_loop
+   ; Warning: this doesn't check if an error is returned!
+ end_init_chain:
  		mov	ax,ending_offset
!                 test    ax,ax
  		jnz	quit_init
  		mov	ax,ending_segment
  		cmp	ax,dev_seg
***************
*** 777,783 ****
  		jb	pr_txt
  check_ver:
  		je	get_list		;DOS v3.xx
! 		cmp	al,5			;ver. above 5 ?
  		ja	pr_txt			;yes,jump
  		jne	dos_v4
  		or	flags,DOS_V5		;DOS v5.xx
--- 799,805 ----
  		jb	pr_txt
  check_ver:
  		je	get_list		;DOS v3.xx
!                 cmp     al,6                    ;ver. above 6 ?
  		ja	pr_txt			;yes,jump
  		jne	dos_v4
  		or	flags,DOS_V5		;DOS v5.xx
***************
*** 1032,1038 ****
  		db	'		Poland',cr,lf
  HELP_LEN	equ	$ - help_scr
  inv_option	db	'IDrv: invalid option! ',cr,lf,lf,'$'
! header		db	'Device Drivers Installer  v1.06',cr,lf
  		db	'(C)  1990  by Janusz Wojcik Lublin,Poland.',cr,lf,lf,'$'
  ;....................................................................................
  LEN		equ	$ - new_seg
--- 1054,1060 ----
  		db	'		Poland',cr,lf
  HELP_LEN	equ	$ - help_scr
  inv_option	db	'IDrv: invalid option! ',cr,lf,lf,'$'
! header          db      'Device Drivers Installer  v1.06p',cr,lf
  		db	'(C)  1990  by Janusz Wojcik Lublin,Poland.',cr,lf,lf,'$'
  ;....................................................................................
  LEN		equ	$ - new_seg
SHAR_EOF
#	End of shell archive
exit 0

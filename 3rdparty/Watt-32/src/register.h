/*!\file register.h
 *
 *  Definition and constants related to Intel CPU.
 *
 *  COPYRIGHT (c) 1998 valette@crf.canon.fr
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.OARcorp.com/rtems/license.html.
 *
 *  $Id: registers.h,v 1.1 1998/08/05 15:15:46 joel Exp $
 */

#ifndef _w32_REGISTER_H
#define _w32_REGISTER_H

/*
 * Definition related to EFLAGS.
 */
#define EFLAGS_CARRY                    0x1
#define EFLAGS_PARITY                   0x4

#define EFLAGS_AUX_CARRY                0x10
#define EFLAGS_ZERO                     0x40
#define EFLAGS_SIGN                     0x80

#define EFLAGS_TRAP                     0x100
#define EFLAGS_INTR_ENABLE              0x200
#define EFLAGS_DIRECTION                0x400
#define EFLAGS_OVERFLOW                 0x800

#define EFLAGS_IOPL_MASK                0x3000
#define EFLAGS_NESTED_TASK              0x8000

#define EFLAGS_RESUME                   0x10000
#define EFLAGS_VIRTUAL_MODE             0x20000
#define EFLAGS_ALIGN_CHECK              0x40000
#define EFLAGS_VIRTUAL_INTR             0x80000

#define EFLAGS_VIRTUAL_INTR_PEND        0x100000
#define EFLAGS_ID                       0x200000

/*
 * Definitions related to CR0.
 */
#define CR0_PROTECTION_ENABLE           0x1
#define CR0_MONITOR_COPROC              0x2
#define CR0_COPROC_SOFT_EMUL            0x4
#define CR0_FLOATING_INSTR_EXCEPTION    0x8

#define CR0_EXTENSION_TYPE              0x10
#define CR0_NUMERIC_ERROR               0x20

#define CR0_WRITE_PROTECT               0x10000
#define CR0_ALIGMENT_MASK               0x40000

#define CR0_NO_WRITE_THROUGH            0x20000000
#define CR0_PAGE_LEVEL_CACHE_DISABLE    0x40000000
#define CR0_PAGING                      0x80000000

#if defined(IN_ASM_FILE)  /* Only set in .S-files used by GCC */
  #if defined(__DJGPP__)
    #include <machine/asm.h>

    /* Use only after ".data" */
    #define DATA(x,type)  .align 2; .globl _##x; _##x: type

  #elif defined(__CYGWIN__) && defined(__x86_64__)
    #define ENTRY(f)      .globl f; f:
    #define DATA(x,type)  .globl x; x: type
    #define _C_LABEL(x)   x

  #elif defined(__x86_64__)
    #define ENTRY(f)      .text; .p2align 8,,15; .globl f; f:
    #define DATA(x,type)  .align 8; .globl x; x: type
    #define _C_LABEL(x)   x

  #else   /* 32-bit GNU assembler */
    #define ENTRY(f)      .text; .p2align 4,,15; .globl _##f; _##f:
    #define DATA(x,type)  .align 4; .globl _##x; _##x: type
    #define _C_LABEL(x)   _##x
  #endif

#else

  /*
   * Definition of eflags registers has a bit field structure.
   */
  typedef struct {
    /*
     * first byte : bits 0->7
     */
    unsigned int carry                : 1;
    unsigned int                      : 1;
    unsigned int parity               : 1;
    unsigned int                      : 1;

    unsigned int auxiliary_carry      : 1;
    unsigned int                      : 1;
    unsigned int zero                 : 1;      /* result is zero */
    unsigned int sign                 : 1;      /* result is less than zero */
    /*
     * Second byte : bits 7->15
     */
    unsigned int trap                 : 1;
    unsigned int intr_enable          : 1;      /* set => intr on */
    unsigned int direction            : 1;      /* set => autodecrement */
    unsigned int overflow             : 1;

    unsigned int IO_privilege         : 2;
    unsigned int nested_task          : 1;
    unsigned int                      : 1;
    /*
     * Third byte : bits 15->23
     */
    unsigned int resume               : 1;
    unsigned int virtual_mode         : 1;
    unsigned int aligment_check       : 1;
    unsigned int virtual_intr         : 1;
    unsigned int virtual_intr_pending : 1;
    unsigned int id                   : 1;
    unsigned int                      : 2;
    /*
     * fourth byte : bits 24->31 : UNUSED
     */
    unsigned int                      : 8;
  } eflags_bits;

  typedef union {
          eflags_bits   eflags;
          unsigned int  i;
        } eflags;

  /*
   * Definition of CR registers has a bit field structure.
   */
  typedef struct {
    /*
     * first byte : bits 0->7
     */
    unsigned int protection_enable        : 1;
    unsigned int monitor_coproc           : 1;
    unsigned int coproc_soft_emul         : 1;
    unsigned int floating_instr_except    : 1;

    unsigned int extension_type           : 1;
    unsigned int numeric_error            : 1;
    unsigned int                          : 2;
    /*
     * second byte 8->15 : UNUSED
     */
    unsigned int                          : 8;
    /*
     * third byte 16->23
     */
    unsigned int write_protect            : 1;
    unsigned int                          : 1;
    unsigned int aligment_mask            : 1;
    unsigned int                          : 1;

    unsigned int                          : 4;
    /*
     * fourth byte 24->31
     */
    unsigned int                          : 4;
    unsigned int                          : 1;
    unsigned int no_write_through         : 1;
    unsigned int page_level_cache_disable : 1;
    unsigned int paging                   : 1;
  } cr0_bits;

  typedef union {
          cr0_bits      cr0;
          unsigned long i;
        } cr0;

#endif  /* IN_ASM_FILE */

#endif


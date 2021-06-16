#include "uboot.h"

// $0       $zero       Hard-wired to 0
// $1       $at         Assembler temporary, reserved for pseudo-instructions
// $2-$3    $v0, $v1    Function return values
// $4-$7    $a0-$a3     Function parameters, not preserved by sub-programs
// $8-$15   $t0-$t7     Temporary data, not preserved by subprograms
// $16-$23  $s0-$s7     Saved registers, preserved by subprograms
// $24-$25  $t8, $t9    Temporary data, not preserved by subprograms
// $26-$27  $k0, $k1    Reserved for interrupt handler. Don't use!
// $28      $gp         Global Area Pointer (base of global data segment)
// $29      $sp         Stack Pointer
// $30      $fp         Frame Pointer
// $31      $ra         Return Address

uboot_header 0x400000, 0x1e000000, 0x3ffff000, "mips64el.rom"

.section .text.entry, "x", @progbits
.global entry
entry:
    ld $10,___image_end_v
    ld $11,___image_vma_v
    ld $12,___image_lma_v


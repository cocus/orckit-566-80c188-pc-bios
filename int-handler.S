.code16

.section .text
bios_ss:
    .word 0xf000
bios_sw_sp:
    .word stack_start

.section .bss
saved_sw_ss: .word 0
saved_sw_sp: .word 0

.macro entry_fn name saved_ss_var saved_sp_var ss_var sp_var
.section .text, "ax"
.globl \name
\name:
    // Save current stack pointer, load BIOS stack
    mov %ss, %cs:\saved_ss_var
    mov %sp, %cs:\saved_sp_var
    mov %cs:\ss_var, %ss
    mov %cs:\sp_var, %sp

    // Save registers, setup stack frame
    sub $2, %sp
    push %es
    push %ds
    pusha
    mov %sp, %bp
    push %bp

    // Load flags from iret frame, store in callregs
    mov %cs:\saved_ss_var, %ds
    mov %cs:\saved_sp_var, %bx
    mov 0(%bx), %ax
    mov 6(%bx), %bx
    mov %bx, 20(%bp)

    // Use SS as DS inside the BIOS for stack-local variables that are passed
    // by address
    mov %ss, %bp
    mov %bp, %ds

    call *%ax
    // Discard regs pointer parameter
    add $2, %sp
    mov %sp, %bp

    // Write possibly updated flags to the iret frame
    mov 20(%bp), %ax
    mov %cs:\saved_ss_var, %ds
    mov %cs:\saved_sp_var, %bx
    mov %ax, 6(%bx)

    // Restore all regs apart from flags - the iret frame has been updated
    // already
    popa
    pop %ds
    pop %es
    add $2, %sp

    // Restore the callers stack
    mov %cs:\saved_ss_var, %ss
    mov %cs:\saved_sp_var, %sp
    add $2, %sp

    iret
.endm

entry_fn irq_entry, saved_sw_ss, saved_sw_sp, bios_ss, bios_sw_sp

.globl unused_int
unused_int:
    cld
    cli
    push $do_unused_int
    jmp irq_entry
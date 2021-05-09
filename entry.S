.code16

// This should land on 0xF000:0xFFF0
.section .reset.text, "ax"
.globl _cpu_reset_entrypoint
_cpu_reset_entrypoint:
    ljmp $0xf000, $_init80c8x_and_start

// This should land on 0xF000:0xFF00
.section .reset.entry.text, "ax"
.globl _init80c8x_and_start
_init80c8x_and_start:
    mov $0xffa4,%dx
    mov $0x8000,%ax
    out %ax,%dx
    jmp _start


.section .entry.text, "ax"
.globl _start
_start:
    mov %cs, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss

    movw $stack_start, %sp

//  SET UP CHIP SELECTS
//
//  UCS#  - EPROM ACCESS
//  LCS#  - SRAM  ACEESS

	mov $0xffa6,%dx     // FINISH SETTING UP UCS#
	mov $0x000e,%ax		// set ISTOP for 0FFFFF ending loaction
	out %al,%dx         // REMEMBER, BYTE WRITE WORK OK

    mov $0xffa0,%dx     // SET UP LCS START REGISTER
	mov $0x0000,%ax     // LCS STARTS AT 0H, ZERO WAIT STATES!
	out %al,%dx         // REMEMBER, BYTE WRITES WORK OK

    mov $0xffa2,%dx     // SET UP LCS STOP REGISTER
    mov $0x200A,%ax     // 128kB Installed
    out %al,%dx         // REMEMBER, BYTE WRITES WORK OK

    // Clear the BIOS stack space
    lea stack_start, %ax
    mov $stack_start,%cx
    sub $stack_bottom,%cx
    mov %ax, %di
    xor %ax, %ax
    std
    rep stosb
    cld

    // Clear bss
    mov $bss_start, %di
    mov $bss_end, %cx
    sub $bss_start, %cx
    mov $0, %al
    rep stosb

    // Initialize rw data
    mov $data_start, %di
    mov $data_end, %cx
    sub %di, %cx
    mov $_data_load, %si
    rep movsb

    jmp root

.pushsection ".bios_date", "a"
    .ascii "11/05/17"
.popsection

.section .model, "a"
    .byte 0xff
    .byte 0x00
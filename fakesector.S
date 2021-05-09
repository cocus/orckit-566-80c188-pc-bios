.code16

.section .text

// sample boot sector bundles on the BIOS code
// just to demonstrate that it boots something

.global fakesector
fakesector:

mov %cs, %ax
mov %ax, %ds
mov %ax, %es
xor %ax, %ax
mov %ax, %ss
mov $0x1fff, %ax
mov %ax, %sp

lea string, %ax
mov %ax, %si
mov  $0x0E, %ah    // teletype output (int 0x10, ah = 0x0E)
print_char:
    lodsb            // al = [ds:si]++
    test %al, %al
    jz   end        // exit if null-terminator found
    int  $0x10        // print character
    jmp  print_char // repeat for next character
end:

mov $0x1000, %bx

mov $0x02, %ah  // read sectors into memory (int 0x13, ah = 0x02)
mov $0x01, %al  // number of sectors to read
mov $0x00, %ch  // track/cylinder
mov $0x01, %cl  // sector number
mov $0x00, %dh  // head number
mov $0x00, %dl  // drive 0
int $0x13

mov $0x0000, %ax
mov %ax, %es
mov $0x1000, %bx
int $0x18

wait_keypress:

xor %ax, %ax
int $0x16
cmp $0, %al
jg got_key
jmp wait_keypress

got_key:
mov $0x0e, %ah  // teletype output (int 0x10, ah = 0x0E)
int $0x10       // print character

jmp wait_keypress

L:
hlt
jmp L



    mov $string, %bp
    mov $0x13, %ah
    mov $0x0a, %bl
    mov $0x01, %al
    mov $12, %cx
    mov $0, %dh
    mov $0, %dl
    int $0x10


string:
    .asciz "From the bl!\n"

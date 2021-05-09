OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS {
    . = 0x0;
    /* with a CS=0xf000, this should land on 0xf0000+0xd000 = 0xfd000 */
    /* with a DS=0xfd00, this should land on 0xfd000+0x0000 = 0xfd000 */
    .text 0 : AT(0xd000) {
        bios_start = . ;
        *(.entry.text);
        
        . = ALIGN(4);
        *(.text);
        *(.text.*);
        . = ALIGN(4);
        vectors_start = . ;
        *(.rodata.vectors);
        vectors_end = . ;
        . = ALIGN(4);
        *(.rodata*);
    }

    _data_load = . ;

    /* with a DS=0xfd00, this should land on 0xfd000+0x5000 = 0x12000 (over 1MiB) */
    .data 0x5000 : AT(0xd000 + SIZEOF(.text)) {
        data_start = . ;
        *(.data*);
        data_end = . ;

        . = ALIGN(4);
        bss_start = . ;
        *(.bss*);
        bss_end = . ;
    }

    /* with a DS=0xfd00, this should land on 0xfd000+0xac00 = 0x17c00 (over 1MiB) */
    .boot_sector 0xac00 : {
        boot_sector_start = . ;
    }

    /* with a DS=0xfd00, this should land on 0xfd000+0xff00 = 0x10ce00 (over 1MiB) */
    .int_stack 0xfe00 : {
        int_stack_bottom = . ;
        . = . + 0x100;
        int_stack_start = . ;
    }

    /* with a DS=0xfd00, this should land on 0xfd000+0xab00 = 0x17b00 (over 1MiB) */
    .stack 0xd000 + SIZEOF(.text) : {
        stack_bottom = . ;
        . = . + 0x100;
        stack_start = . ;
    }

    /* with a CS=0xf000, this should land on 0xf0000+0xfff0 = 0xffff0 */
    .reset : AT(0xfff0) { 
        *(.reset.text);
        FILL(0xdead);
        . = . + 4;
    }

    /* Throw away everything else. */
    /DISCARD/ : { *(*) }
}
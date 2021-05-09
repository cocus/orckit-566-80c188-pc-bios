OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS {
    . = 0x0;
    /* with a CS=0xf000, this should land on 0xf0000+0xd000 = 0xfd000 */
    .text 0xd000 : {
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

    /* with a DS=0xf000, this should land on 0xf0000+0xc000 = 0xfc000 */
    .data 0xc000 : AT(_data_load) {
        stack_bottom = . ;
        . = . + 0x100;
        stack_start = . ;

        data_start = . ;
        *(.data*);
        data_end = . ;

        . = ALIGN(4);
        bss_start = . ;
        *(.bss*);
        bss_end = . ;
    }

    .entryjump 0xff00 : AT(0xff00) {
        *(.reset.entry.text);
        FILL(0xf0f0);
    }

    /* with a CS=0xf000, this should land on 0xf0000+0xfff0 = 0xffff0 */
    .reset 0xfff0 : AT(0xfff0) { 
        *(.reset.text);
    }

    .bios_date 0xfff6 : AT(0xfff6) {
        *(.bios_date);
    }

    .model 0xfffe : AT(0xfffe) {
        *(.model);
    }

    /* Throw away everything else. */
    /DISCARD/ : { *(*) }
}
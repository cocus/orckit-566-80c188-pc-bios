

OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386)
ENTRY(fakesector)

SECTIONS {
    . = 0x0;
    .text 0x7c00 : {
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

    /* with a DS=0xfd00, this should land on 0xfd000+0x4000 = 0x11000 (over 1MiB) */
    .data : {
        data_start = . ;
        *(.data*);
        data_end = . ;

        . = ALIGN(4);
        bss_start = . ;
        *(.bss*);
        bss_end = . ;
    }

    /* Throw away everything else. */
    /DISCARD/ : { *(*) }
}

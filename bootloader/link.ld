OUTPUT_FORMAT("pei-x86-64")
ENTRY(efi_main)
SECTIONS {
    . = 0;
    .text : {
        *(.text*)
    }
    .data : {
        *(.data*)
    }
    .bss : {
        *(.bss*)
        *(COMMON)
    }
}

/* OceanOS UEFI Bootloader Linker Script */

OUTPUT_FORMAT("pei-x86-64")
ENTRY(efi_main)

SECTIONS {
    /* Start at virtual address 0 */
    . = 0;
    
    /* PE/COFF header */
    .header : {
        KEEP(*(.header))
    }
    
    /* Text section - executable code */
    .text : {
        *(.text)
        *(.text.*)
        *(.gnu.linkonce.t.*)
    }
    
    /* Read-only data */
    .rodata : {
        *(.rodata)
        *(.rodata.*)
        *(.gnu.linkonce.r.*)
    }
    
    /* Data section - initialized data */
    .data : {
        *(.data)
        *(.data.*)
        *(.gnu.linkonce.d.*)
    }
    
    /* BSS section - uninitialized data */
    .bss : {
        *(.bss)
        *(.bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
    }
    
    /* Relocation information */
    .reloc : {
        *(.reloc)
    }
    
    /* Export table */
    .edata : {
        *(.edata)
    }
    
    /* Import table */
    .idata : {
        *(.idata)
    }
    
    /* Discard sections */
    /DISCARD/ : {
        *(.comment)
        *(.gnu.*)
        *(.note.*)
        *(.eh_frame)
        *(.eh_frame_hdr)
    }
}

/* Symbols for bootloader */
bootloader_start = .;
bootloader_end = .;
bootloader_size = bootloader_end - bootloader_start;

/* OceanOS Kernel Linker Script */

ENTRY(kernel_main)

/* Memory layout */
KERNEL_VIRTUAL_BASE = 0xFFFFFFFF80000000;
KERNEL_PHYSICAL_BASE = 0x100000;

SECTIONS {
    /* Start at 1MB physical address */
    . = KERNEL_PHYSICAL_BASE;
    
    /* Kernel header */
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
        *(.bSS)
        *(.bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
    }
    
    /* Stack section */
    .stack : {
        . = ALIGN(16);
        . += 0x1000; /* 4KB stack */
        stack_top = .;
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

/* Symbols for kernel */
kernel_start = KERNEL_PHYSICAL_BASE;
kernel_end = .;
kernel_size = kernel_end - kernel_start;

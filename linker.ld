ENTRY(start)
KERNEL_START_PMA = 0x00100000;
SECTIONS {
	. = KERNEL_START_PMA;
	.text ALIGN (0x1000) :
	{
        *(.multiboot)
		*(.text)
	}
    
    .rodata :
    {
    	*(.rodata)
    }

    .data :
    {
    	*(.data)
    }

    .bss :
    {
    	*(.bss)
    }
}


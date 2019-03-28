ENTRY(start)
SECTIONS {
	. = 0x00100000;
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


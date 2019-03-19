ENTRY(start)
SECTIONS {
	. = 0x00100000;
	.text ALIGN (0x1000) :
	{
        *(.boot)
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


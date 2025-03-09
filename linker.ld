OUTPUT_FORMAT(binary)
SECTIONS
{
	. = 0x7C00;
	.text :
	{
		*(.text)
	}
	.rodata :
	{
		*(.rodata)
	}
	.data :
	{
		*(.data)
		. = ALIGN(512);
	}
	.bss :
	{
		*(COMMON)
		*(.bss)
	}
}

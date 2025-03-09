OUTPUT_FORMAT(binary)
SECTIONS
{
	. = 0x7C00;
	image_start = .;
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
	image_end = .;
	sector_count = (image_end - image_start) >> 9;
	.bss :
	{
		*(COMMON)
		*(.bss)
	}
}

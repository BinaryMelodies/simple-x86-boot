
; Usage:
;	_descriptor	base, limit, attributes
%macro	_descriptor	3
	; Segment descriptors use an unusual layout
	dw	(%2) & 0xFFFF
	dw	(%1) & 0xFFFF
	dw	(((%1) >> 16) & 0x00FF) | (((%3) & 0x00FF) << 8)
	dw	(((%2) >> 16) & 0x000F) | (((%3) & 0xF000) >> 8) | (((%1) >> 16) & 0xFF00)
%endmacro

; Usage:
;	descriptor	base, limit, attributes
%macro	descriptor	3
	; A more convenient way to handle segments that are larger than 64KiB
%if	((%3) & 0x8000) || (%2) > 0xFFFF
	_descriptor	%1, (%2) >> 12, (%3) | 0x8000
%else
	_descriptor	%1, %2, (%3) & 0x7FFF
%endif
%endmacro

%define	DESC_CODE  0x009A
%define	DESC_DATA  0x0092
%define DESC_CPL0  0x0000
%define DESC_CPL3  0x0060
%define DESC_16BIT 0x0000
%define DESC_32BIT 0x4000
%define DESC_64BIT 0x2000 ; only needed for the code segment

	section	.text

	bits	16

	; Clear CS, some BIOSes jump to a different address but the same memory location
	jmp	0:rm_start
rm_start:
	; Set up stack right below boot code
	xor	ax, ax
	mov	ss, ax
	mov	sp, 0x7C00
	; Make DS = CS for easier access
	mov	ds, ax

%ifdef OS286
	; Turn off interrupts while setting up protected mode
	cli
	; The GDTR is needed to set up protected mode segments
	lgdt	[gdtr]
	; Enter protected mode in a 286 compatible way, there's no CR0, only the MSW which contains the low bits
	smsw	ax
	or	al, 1
	lmsw	ax
	; Load CS with a protected mode descriptor
	jmp	0x08:pm_start
%elifdef OS386
	; Turn off interrupts while setting up protected mode
	cli
	lgdt	[gdtr]
	; Enter protected mode
	mov	eax, cr0
	or	al, 1
	mov	cr0, eax
	; Load CS with a protected mode descriptor
	jmp	0x08:pm_start
%elifdef OS64
	; Entering long mode directly from real mode, based on https://wiki.osdev.org/Entering_Long_Mode_Directly
	; Set up identity paging tables from 0x1000
	mov	di, 0x1000
	push	di
	mov	ecx, 0x1000
	xor	eax, eax
	mov	es, ax
	cld
	rep stosd
	pop	di

	push	di
	mov	cx, 3
.setup_directories:
	lea	eax, [di + 0x1000]
	or	ax, 3
	mov	[di], eax
	add	di, 0x1000
	loop	.setup_directories

	mov	eax, 3
	mov	ecx, 0x200
.setup_id_paging:
	mov	[di], eax
	add	eax, 0x1000
	add	di, 8
	loop	.setup_id_paging

	xor	edi, edi
	pop	di

	; Turn off interrupts while setting up protected mode
	cli
	; The GDTR is needed to set up protected mode segments
	lgdt	[gdtr]
	; Enable paging extensions and PAE
	mov	eax, 0x000000a0
	mov	cr4, eax
	; Load page tables
	mov	cr3, edi
	; Enable long mode
	mov	ecx, 0xC0000080
	rdmsr
	or	ax, 0x0100
	wrmsr
	; Enter protected mode and enable paging
	mov	eax, cr0
	or	eax, 0x80000001
	mov	cr0, eax
	; Load CS with a protected mode descriptor
	jmp	0x08:pm_start
%endif

%ifdef OS86
	; Write message
	; The VGA text buffer starts at 0xB8000
	mov	ax, 0xB800
	mov	es, ax
	xor	di, di
	mov	si, message
	mov	cx, message_length
	mov	ah, 0x1E
%elifdef OS286
pm_start:
	; Now we are in protected mode
	; Set up stack and other segment registers with protected mode selectors
	mov	sp, 0x7C00
	mov	ax, 0x10
	mov	ss, ax
	mov	ds, ax
	mov	es, ax

	; Write message
	; The VGA text buffer is accessed via a different selector from the kernel DS
	mov	ax, 0x18
	mov	es, ax
	xor	di, di
	mov	si, message
	mov	cx, message_length
	mov	ah, 0x1E
%elifdef OS386
	bits	32
pm_start:
	; Now we are in 32-bit protected mode
	; Set up stack and other segment registers with protected mode selectors
	mov	esp, 0x7C00
	mov	ax, 0x10
	mov	ss, ax
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	mov	gs, ax

	; Write message
	; The VGA text buffer starts at 0xB8000
	mov	edi, 0xB8000
	mov	esi, message
	mov	ecx, message_length
	mov	ah, 0x1E
%elifdef OS64
	bits	64
pm_start:
	; Now we are in 64-bit protected mode
	; Set up stack and other segment registers with protected mode selectors
	mov	esp, 0x7C00
	mov	ax, 0x10
	mov	ss, ax
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	mov	gs, ax

	; Write message
	; The VGA text buffer starts at 0xB8000
	mov	edi, 0xB8000
	mov	esi, message
	mov	ecx, message_length
	mov	ah, 0x1E
%endif

.1:
	lodsb
	stosw
	loop	.1

	; Halt
	cli
.2:
	hlt
	jmp	.2

%ifdef OS286
	align	4, db 0
gdtr:
	dw	gdt_end - gdt - 1
	dd	gdt

	align	8, db 0
gdt:
	dw	0, 0, 0, 0
	descriptor	0, 0xFFFF, DESC_CODE | DESC_16BIT
	descriptor	0, 0xFFFF, DESC_DATA | DESC_16BIT
	; A separate selector to access the VGA text buffer
	descriptor	0xB8000, 0xFFFF, DESC_DATA | DESC_16BIT
gdt_end:
%elifdef OS386
	align	4, db 0
gdtr:
	dw	gdt_end - gdt - 1
	dd	gdt

	align	8, db 0
gdt:
	dw	0, 0, 0, 0
	descriptor	0, 0xFFFFFFFF, DESC_CODE | DESC_32BIT
	descriptor	0, 0xFFFFFFFF, DESC_DATA | DESC_32BIT
gdt_end:
%elifdef OS64
	align	4, db 0
gdtr:
	dw	gdt_end - gdt - 1
	dd	gdt

	align	8, db 0
gdt:
	dw	0, 0, 0, 0
	; 64-bit descriptors don't need a limit, and only the code segment needs to be set to 64-bit
	descriptor	0, 0, DESC_CODE | DESC_64BIT
	descriptor	0, 0, DESC_DATA
gdt_end:
%endif

message:
	db	"Greetings! "
%ifdef OS86
	db	"OS/86 running in real mode (8086)"
%elifdef OS286
	db	"OS/286 running in 16-bit protected mode (80286)"
%elifdef OS386
	db	"OS/386 running in 32-bit protected mode (80386)"
%elifdef OS64
	db	"OS/64 running in 64-bit long mode"
%else
	db	"OS/???"
%endif
message_length	equ	$ - message

	section	.data


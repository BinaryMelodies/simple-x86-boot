# Simple boot code for x86 based systems

This is a single sector boot code that can be compiled to run in 16-bit real mode, 16-bit and 32-bit protected mode and 64-bit long mode.

To compile all 4 versions:

> make

To run all 4 versions:

> ./run 16
> ./run pm
> ./run 32
> ./run 64

Requirements:

* Netwide Assembler
* Make
* QEMU system emulator for 386 and x86-64


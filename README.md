# Simple boot code for x86 based systems

This is a simple bootable code that can be compiled to run in 16-bit real mode, 16-bit and 32-bit protected mode and 64-bit long mode.

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
* Bare metal GCC compilers for ia16, i686 and x86_64
* QEMU system emulator for i386 and x86_64

The code was inspired by [James Molloy's tutorial](http://www.jamesmolloy.co.uk/tutorial_html/index.html) and the [OSDev Wiki](https://wiki.osdev.org/).


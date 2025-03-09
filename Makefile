
all: 8086.img 286.img 386.img x86-64.img

clean:
	rm -rf *.img obj

distclean: clean
	rm -rf *~

obj/8086/boot.o: boot.asm
	mkdir -p `dirname $@`
	nasm -felf $< -o $@ -DOS86

obj/8086/kernel.o: kernel.c
	mkdir -p `dirname $@`
	ia16-elf-gcc -c $< -o $@ -DOS86=1 -std=gnu99 -ffreestanding -O2 -Wall -Wextra

obj/286/boot.o: boot.asm
	mkdir -p `dirname $@`
	nasm -felf $< -o $@ -DOS286

obj/286/kernel.o: kernel.c
	mkdir -p `dirname $@`
	ia16-elf-gcc -c $< -o $@ -DOS286=1 -std=gnu99 -ffreestanding -O2 -Wall -Wextra -mprotected-mode

obj/386/boot.o: boot.asm
	mkdir -p `dirname $@`
	nasm -felf $< -o $@ -DOS386

obj/386/kernel.o: kernel.c
	mkdir -p `dirname $@`
	i686-elf-gcc -c $< -o $@ -DOS386=1 -std=gnu99 -ffreestanding -O2 -Wall -Wextra

obj/x86-64/boot.o: boot.asm
	mkdir -p `dirname $@`
	nasm -felf64 $< -o $@ -DOS64

obj/x86-64/kernel.o: kernel.c
	mkdir -p `dirname $@`
	x86_64-elf-gcc -c $< -o $@ -DOS64=1 -std=gnu99 -ffreestanding -O2 -Wall -Wextra -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2

obj/8086/image.bin: obj/8086/boot.o obj/8086/kernel.o
	ld -melf_i386 -T linker.ld -o $@ $^

obj/286/image.bin: obj/286/boot.o obj/286/kernel.o
	ld -melf_i386 -T linker.ld -o $@ $^

obj/386/image.bin: obj/386/boot.o obj/386/kernel.o
	ld -melf_i386 -T linker.ld -o $@ $^

obj/x86-64/image.bin: obj/x86-64/boot.o obj/x86-64/kernel.o
	ld -melf_x86_64 -T linker.ld -o $@ $^

8086.img: obj/8086/image.bin
	dd if=/dev/zero of=$@ count=1440 bs=1024
	dd if=$< of=$@ conv=notrunc
	python3 makeboot.py $@

286.img: obj/286/image.bin
	dd if=/dev/zero of=$@ count=1440 bs=1024
	dd if=$< of=$@ conv=notrunc
	python3 makeboot.py $@

386.img: obj/386/image.bin
	dd if=/dev/zero of=$@ count=1440 bs=1024
	dd if=$< of=$@ conv=notrunc
	python3 makeboot.py $@

x86-64.img: obj/x86-64/image.bin
	dd if=/dev/zero of=$@ count=1440 bs=1024
	dd if=$< of=$@ conv=notrunc
	python3 makeboot.py $@

.PHONY: all clean distclean


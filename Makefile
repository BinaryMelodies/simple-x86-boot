
all: 8086.img 286.img 386.img x86-64.img

clean:
	rm -rf *.img obj

distclean: clean
	rm -rf *~ src/*~

obj/8086/boot.o: src/boot.asm
	mkdir -p `dirname $@`
	nasm -felf $< -o $@ -DOS86

obj/8086/kernel.o: src/kernel.c
	mkdir -p `dirname $@`
	#ia16-elf-gcc -c $< -o $@ -DOS86=1 -std=gnu99 -ffreestanding -O2 -Wall -Wextra -fno-delete-null-pointer-checks
	ia16-elf-gcc -c $< -o $@ -DOS86=1 -std=gnu99 -ffreestanding -Wall -Wextra -fno-delete-null-pointer-checks

obj/286/boot.o: src/boot.asm
	mkdir -p `dirname $@`
	nasm -felf $< -o $@ -DOS286

obj/286/kernel.o: src/kernel.c
	mkdir -p `dirname $@`
	#ia16-elf-gcc -c $< -o $@ -DOS286=1 -std=gnu99 -ffreestanding -O2 -Wall -Wextra -march=i80286 -mprotected-mode
	ia16-elf-gcc -c $< -o $@ -DOS286=1 -std=gnu99 -ffreestanding -Wall -Wextra -march=i80286 -mprotected-mode

obj/386/boot.o: src/boot.asm
	mkdir -p `dirname $@`
	nasm -felf $< -o $@ -DOS386

obj/386/kernel.o: src/kernel.c
	mkdir -p `dirname $@`
	i686-elf-gcc -c $< -o $@ -DOS386=1 -std=gnu99 -ffreestanding -O2 -Wall -Wextra -march=i386

obj/x86-64/boot.o: src/boot.asm
	mkdir -p `dirname $@`
	nasm -felf64 $< -o $@ -DOS64

obj/x86-64/kernel.o: src/kernel.c
	mkdir -p `dirname $@`
	x86_64-elf-gcc -c $< -o $@ -DOS64=1 -std=gnu99 -ffreestanding -O2 -Wall -Wextra -march=x86-64 -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2

obj/8086/kernel.elf: obj/8086/boot.o obj/8086/kernel.o
	ia16-elf-gcc -T src/linker.ld -o $@ -ffreestanding -O2 -nostdlib $^ -lgcc

obj/8086/kernel.bin: obj/8086/kernel.elf
	objcopy -Obinary $< $@

obj/286/kernel.elf: obj/286/boot.o obj/286/kernel.o
	ia16-elf-gcc -T src/linker.ld -o $@ -ffreestanding -O2 -nostdlib $^ -lgcc

obj/286/kernel.bin: obj/286/kernel.elf
	objcopy -Obinary $< $@

obj/386/kernel.elf: obj/386/boot.o obj/386/kernel.o
	i686-elf-gcc -T src/linker.ld -o $@ -ffreestanding -O2 -nostdlib $^ -lgcc

obj/386/kernel.bin: obj/386/kernel.elf
	objcopy -Obinary $< $@

obj/x86-64/kernel.elf: obj/x86-64/boot.o obj/x86-64/kernel.o
	x86_64-elf-gcc -T src/linker.ld -o $@ -ffreestanding -O2 -nostdlib $^ -lgcc

obj/x86-64/kernel.bin: obj/x86-64/kernel.elf
	objcopy -Obinary $< $@

8086.img: obj/8086/kernel.bin
	dd if=/dev/zero of=$@ count=1440 bs=1024
	dd if=$< of=$@ conv=notrunc
	python3 src/makeboot.py $@

286.img: obj/286/kernel.bin
	dd if=/dev/zero of=$@ count=1440 bs=1024
	dd if=$< of=$@ conv=notrunc
	python3 src/makeboot.py $@

386.img: obj/386/kernel.bin
	dd if=/dev/zero of=$@ count=1440 bs=1024
	dd if=$< of=$@ conv=notrunc
	python3 src/makeboot.py $@

x86-64.img: obj/x86-64/kernel.bin
	dd if=/dev/zero of=$@ count=1440 bs=1024
	dd if=$< of=$@ conv=notrunc
	python3 src/makeboot.py $@

.PHONY: all clean distclean


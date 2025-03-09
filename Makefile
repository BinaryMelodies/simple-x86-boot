
all: 8086.img 286.img 386.img x86-64.img

clean:
	rm -rf *.img obj

distclean: clean
	rm -rf *~

obj/8086/boot.o: boot.asm
	mkdir -p `dirname $@`
	nasm -felf $< -o $@ -DOS86

obj/286/boot.o: boot.asm
	mkdir -p `dirname $@`
	nasm -felf $< -o $@ -DOS286

obj/386/boot.o: boot.asm
	mkdir -p `dirname $@`
	nasm -felf $< -o $@ -DOS386

obj/x86-64/boot.o: boot.asm
	mkdir -p `dirname $@`
	nasm -felf64 $< -o $@ -DOS64

obj/8086/boot.bin: obj/8086/boot.o
	ld -melf_i386 -T linker.ld -o $@ $<

obj/286/boot.bin: obj/286/boot.o
	ld -melf_i386 -T linker.ld -o $@ $<

obj/386/boot.bin: obj/386/boot.o
	ld -melf_i386 -T linker.ld -o $@ $<

obj/x86-64/boot.bin: obj/x86-64/boot.o
	ld -melf_x86_64 -T linker.ld -o $@ $<

8086.img: obj/8086/boot.bin
	dd if=/dev/zero of=$@ count=1440 bs=1024
	dd if=$< of=$@ conv=notrunc
	python3 makeboot.py $@

286.img: obj/286/boot.bin
	dd if=/dev/zero of=$@ count=1440 bs=1024
	dd if=$< of=$@ conv=notrunc
	python3 makeboot.py $@

386.img: obj/386/boot.bin
	dd if=/dev/zero of=$@ count=1440 bs=1024
	dd if=$< of=$@ conv=notrunc
	python3 makeboot.py $@

x86-64.img: obj/x86-64/boot.bin
	dd if=/dev/zero of=$@ count=1440 bs=1024
	dd if=$< of=$@ conv=notrunc
	python3 makeboot.py $@

.PHONY: all clean distclean


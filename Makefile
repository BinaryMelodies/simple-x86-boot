
all: 8086.img 286.img 386.img x86-64.img

clean:
	rm -rf *.img

distclean: clean
	rm -rf *~

8086.img: boot.asm
	nasm -fbin $< -o $@ -DOS86

286.img: boot.asm
	nasm -fbin $< -o $@ -DOS286

386.img: boot.asm
	nasm -fbin $< -o $@ -DOS386

x86-64.img: boot.asm
	nasm -fbin $< -o $@ -DOS64

.PHONY: all clean distclean


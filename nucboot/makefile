all: boot_fd.bin

boot_fd.bin boot_fd.lst: boot_fd.asm makefile
	nasm -O9 -f bin -l boot_fd.lst -o boot_fd.bin boot_fd.asm

clean:
	-rm boot_fd.bin boot_fd.lst

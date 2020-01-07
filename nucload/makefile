
CFLAGS = -fno-builtin -Os

all: nucload.sys

clean:
	-rm nucload.sys nucload.o nucloada.o nucload.lst

nucload.o: nucload.c
	gcc $(CFLAGS) -mtune=i386 -o nucload.o -c nucload.c

nucloada.o: nucloada.asm
	nasm -w+orphan-labels -O9 -f coff -l nucloada.lst -o nucloada.o nucloada.asm

nucload.sys: nucload.o nucloada.o nucload.ld
	ld --oformat binary -M -T nucload.ld -o nucload.sys nucload.o nucloada.o > nucload.map

FORCE:

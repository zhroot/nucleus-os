
# v0.1: ???
# v0.2: Doug Gale
#	- Major rewrite
#	- Automatically pulls in all video driver sources
#	- Creates detailed assembly, annotated with source
#	- Creates link map file
#	- Dumps all debugging information to text file
#	- Support for automatically generated dependencies!
#	- Support to build bochs compatible symbol file
#	- Added variable to allow include paths to be added easily

# Compiler, linker, assembler
CC = gcc
LD = ld
AS = nasm

# Preprocessor stuff
INCLUDE_DIRS += include
INCLUDE_FLAGS = $(patsubst %,-I%,$(INCLUDE_DIRS))

# Automatically pull in all driver sources
#  (When everything works...)
#DRV_SRCS += $(wildcard drivers/*.c)
#DRV_SRCS += $(wildcard drivers/cpu/*.asm)
#DRV_SRCS += $(wildcard drivers/cpu/*.c)
#DRV_SRCS += $(wildcard drivers/mem/*.asm)
#DRV_SRCS += $(wildcard drivers/mem/*.c)
#DRV_SRCS += $(wildcard drivers/input/*.asm)
#DRV_SRCS += $(wildcard drivers/input/*.c)
#DRV_SRCS += $(wildcard video/*.asm)
#DRV_SRCS += $(wildcard video/*.c)
DRV_SRCS += $(wildcard video/*.c)
DRV_SRCS += $(wildcard video/*.asm)
#DRV_SRCS += $(wildcard drivers/block/*.asm)
#DRV_SRCS += $(wildcard drivers/block/*.c)
#DRV_SRCS += $(wildcard drivers/fs/*.asm)
#DRV_SRCS += $(wildcard drivers/fs/*.c)
#
DRV_OBJ2 = $(DRV_SRCS)
DRV_OBJ1 = $(DRV_OBJ2:.c=.o)
DRV_OBJ = $(DRV_OBJ1:.asm=.o)

# Options
LDFLAGS = --oformat binary
ASFLAGS = -O9 -f coff -g
CFLAGS = $(INCLUDE_FLAGS) $(WARNFLAGS) -fno-builtin -g -DDEBUG -O -mtune=athlon
WARNFLAGS = -Wall -Wsystem-headers

# Strong warning - enforce prototypes for global functions:
# -Wmissing-prototypes -Werror

CPUOBJ = drivers/cpu/cpu.o drivers/cpu/cpu_k6.o drivers/cpu/cpu_mhz.o drivers/cpu/bugs.o
INPUTOBJ = drivers/input/keyboard.o drivers/input/keymaps.o drivers/input/layout.o
BLOCKOBJ = drivers/block/floppy.o drivers/block/ide.o drivers/fs/fat.o

DRIVEROBJ = drivers/pic.o \
	drivers/mem/mem.o \
	drivers/timer.o \
	drivers/input.o \
	drivers/block.o \
	drivers/cmos.o \
	drivers/dma.o \
	drivers/pci.o

MAINOBJ = header.o \
	nucleus.o \
	support.o \
	excepta.o \
	except.o \
	pagetbl.o \
	gdt.o \
	tss.o \
	stdio.o \
	logo.o \
	strings.o \
	string.o \
	interrupts.o \
	irqa.o \
	multi.o \
	dynlink.o \
	cdsl.o \
	stss.o

OBJ = $(MAINOBJ) $(DRIVEROBJ) $(CPUOBJ) $(DRV_OBJ) $(INPUTOBJ) $(BLOCKOBJ)

all: nucleus.bin
	@rem echo Nucleus was successfully compiled!

clean:
	-rm $(OBJ)
	-rm $(OBJ:.o=.dep)
	-rm nucleus.bin nucleus.dmp nucleus.map
	-rm nucleus.cof nucleus.sym nucleus.syb
	@rem echo Nucleus was deleted!

showinputs:
	@echo $(OBJ)

nucleus.bin: $(OBJ) nucleus.ld $(OBJ:.o=.dep)
	$(LD) $(OBJ) $(LDFLAGS) -o $@ -T nucleus.ld

%.o: %.asm
	$(AS) $(ASFLAGS) $? -o $@

%.dep: %.asm
	$(AS) -M $(ASFLAGS) $< > $@

%.dep: %.c
	$(CC) -M $(CFLAGS) -o $@ $<

# Pull in generated dependencies
-include $(OBJ:.o=.dep)

nucleus.cof: nucleus.bin $(OBJ) nucldbg.ld
	$(LD) $(OBJ) $(LDFLAGS) --oformat coff-go32 -M -g -o nucleus.cof -T nucldbg.ld > nucleus.map

nucleus.dmp: nucleus.cof
	objdump -d -S --show-raw-insn nucleus.cof > nucleus.dmp
#	objdump --debugging  nucleus.cof > nucleus.dmp

nucleus.sym: nucleus.cof
	objdump --syms nucleus.cof > nucleus.sym

nucleus.syb: nucleus.sym
	perl stripsyms.pl < nucleus.sym > nucleus.syb
	sort nucleus.syb /O nucleus.syb

disasm: nucleus.dmp nucleus.syb

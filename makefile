
all: nucleus nucload nucboot nucflop FORCE
	@cd temp
	@nucflop --nucinst

nucleus: FORCE
	@cd nucleus
	@make disasm
	@cd ..
	@cp nucleus/nucleus.bin temp

nucboot: FORCE
	@cd nucboot
	@make
	@cd ..
	@cp nucboot/boot_fd.bin temp

nucload: FORCE
	@cd nucload
	@make
	@cd ..
	@cp nucload/nucload.sys temp

nucflop: FORCE
	@cd nucflop
	@make
	@cd ..
	@cp nucflop/nucflop.exe temp

clean:
	@cd nucleus
	@make clean
	@cd ..
	@cd nucboot
	@make clean
	@cd ..
	@cd nucload
	@make clean
	@cd ..
	@cd nucflop
	@make clean
	@cd ..

FORCE:


CCPRGM ?= i686-linux-gnu-gcc
ASPRGM ?= i686-linux-gnu-as
LDPRGM ?= i686-linux-gnu-ld
STRIPPRGM ?= i686-linux-gnu-strip
OBJCOPYPRGM ?= i686-linux-gnu-objcopy

CFLAGS ?= -std=c99 -Wpedantic -m32 -march=i386 -nostartfiles -nostdlib -nodefaultlibs -static -c
ASFLAGS ?= -march=i386
LDFLAGS ?= --no-dynamic-linker -Ttext=0x0

build StallOS/stallos.bin: build/stall.bin
	cp build/stall.bin StallOS/stallos.bin

build/stall.bin: build/stall.elf build/kernel.bin build/prgm.elf build/shell.elf
	${OBJCOPYPRGM} --dump-section .text=build/stall.bin build/stall.elf /dev/null
	dd if=build/kernel.bin of=build/stall.bin bs=512 skip=110 seek=48
	dd if=build/prgm.elf of=build/stall.bin bs=512 seek=66
	dd if=build/shell.elf of=build/stall.bin bs=512 seek=194
	dd if=/dev/zero of=build/stall.bin bs=512 count=1 seek=2879

run: StallOS/stallos.bin
	./run.sh

build/shell.elf: build/shell-asm.elf build/shell-ul.elf
	${LDPRGM} ${LDFLAGS} -o build/shell.elf build/shell-ul.elf build/shell-asm.elf -lgcc

build/shell-asm.elf: os/shell.s
	${ASPRGM} ${ASFLAGS} -o build/shell-asm.elf os/shell.s

build/shell-ul.elf: os/shell.c
	${CCPRGM} ${CFLAGS} -o build/shell-ul.elf os/shell.c

build/prgm.elf: build/prgm-ul.elf build/prgm-asm.elf
	${LDPRGM} ${LDFLAGS} -o build/prgm.elf build/prgm-ul.elf build/prgm-asm.elf -lgcc

build/prgm-asm.elf: build/system-comp.s
	${ASPRGM} ${ASFLAGS} -o build/prgm-asm.elf build/system-comp.s

build/system-comp.s: os/system.s os/irupt_generic.s
	cp os/system.s build/system-comp.s
	awk '1{gsub(/NUM/, thenum, $$0);print($$0);}' thenum=70 os/irupt_generic.s thenum=71 os/irupt_generic.s thenum=72 os/irupt_generic.s thenum=73 os/irupt_generic.s thenum=74 os/irupt_generic.s thenum=75 os/irupt_generic.s thenum=76 os/irupt_generic.s thenum=77 os/irupt_generic.s thenum=78 os/irupt_generic.s thenum=79 os/irupt_generic.s thenum=7a os/irupt_generic.s thenum=7b os/irupt_generic.s thenum=7c os/irupt_generic.s thenum=7d os/irupt_generic.s thenum=7e os/irupt_generic.s thenum=7f os/irupt_generic.s >> build/system-comp.s

build/prgm-ul.elf: os/system.c os/kernel/*.h
	${CCPRGM} ${CFLAGS} -o build/prgm-ul.elf os/system.c

build/kernel.bin: build/kernel.o build/sysc.elf build/irupts.o
	${LDPRGM} ${LDFLAGS} -o build/kernel.elf build/kernel.o build/sysc.elf build/irupts.o -lgcc
	cp build/kernel.elf build/kernel-copy.elf
	${STRIPPRGM} build/kernel-copy.elf	
	${OBJCOPYPRGM} --dump-section .text=build/kernel.bin build/kernel.elf /dev/null

build/sysc.elf: sys32/sys.c
	${CCPRGM} ${CFLAGS} -o build/sysc.elf sys32/sys.c

build/kernel.o: build/kernel-comp.s
	${ASPRGM} ${ASFLAGS} -o build/kernel.o build/kernel-comp.s

build/irupts.o: sys32/irupts.s
	${ASPRGM} ${ASFLAGS} -o build/irupts.o sys32/irupts.s

build/kernel-comp.s: sys32.dhulb
	dhulbpp - - < sys32.dhulb > build/kern32-comp.dhulb
	dhulbc 32 -tNGT < build/kern32-comp.dhulb > build/kernel-comp.s

build/stall.elf: build/stall.o
	${LDPRGM} ${LDFLAGS} -o build/stall.elf build/stall.o -lgcc

build/stall.o: build/stall-comp.s
	${ASPRGM} ${ASFLAGS} -o build/stall.o build/stall-comp.s

build/stall-comp.s: build/Salth.class stall.slth sys16.dhulb stall.s shell.s kernel/int.s
	cp stall.s build/stall-comp.s
	java -cp build Salth n staltstd < stall.slth >> build/stall-comp.s
	printf ".if staltstd_str_commandline_addr\n  .err # The command line address is offset from the start of the shell's static text segment\n.endif\n" >> build/stall-comp.s
	cat sys16.dhulb | dhulbpp - - | dhulbc 16 -tNT >> build/stall-comp.s
	cat ${DHULB_PATH}/src/DLib/pc/io.s ${DHULB_PATH}/src/DLib/util_16.s shell.s ${DHULB_PATH}/src/DLib/stall/stack.s ${DHULB_PATH}/src/DLib/stall/sys.s ${DHULB_PATH}/src/DLib/dos/api_bindings.s kernel/int.s >> build/stall-comp.s
	# TODO Make this rule run when any of the used "DLib" things have been updated


build/Salth.class: Salth.java
	javac -d build Salth.java

clean:
	rm -f build/*


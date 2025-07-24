
FONTES = boot.s switch.s sched.c main.c
RPICPU = bcm2836
LDSCRIPT = linker.ld
PROJETO = tasks

# Interface serial a utilizar
TTY = /dev/ttyUSB0

# Configuração JTAG
OCD_CFG = rpi2-ftdi.cfg

#
# Arquivos de saída 
#
EXEC = ${PROJETO}.elf
MAP = ${PROJETO}.map
LIST = ${PROJETO}.list
IMAGE = ${PROJETO}.img
HEXFL = ${PROJETO}.hex

PREFIXO = arm-none-eabi-
AS = ${PREFIXO}as
LD = ${PREFIXO}ld
GCC = ${PREFIXO}gcc
OBJCPY = ${PREFIXO}objcopy
OBJDMP = ${PREFIXO}objdump
OBJ = $(FONTES:.s=.o)
OBJETOS = $(OBJ:.c=.o)

PATH_GCC = /usr/lib/gcc/arm-none-eabi/13.2.1
PATH_NEWLIB = /usr/lib/arm-none-eabi/newlib

ifeq (${RPICPU}, bcm2836)
	# Raspberry Pi v.2 ou v.3
	ASMOPTIONS = -g --defsym RPICPU=2
	COPTIONS = -march=armv7-a -mtune=cortex-a7 -g -D RPICPU=2
	LDOPTS = -lgcc -L${PATH_GCC} -L${PATH_NEWLIB} -lc_nano
else
	ifeq (${RPICPU}, bcm2835)
  		# Raspberry Pi v.0 ou v.1
   	ASMOPTIONS = -march=armv6zk -g --defsym RPICPU=0
   	COPTIONS = -march=armv6zk -mtune=arm1176jzf-s -g -D RPICPU=0
		LDOPTS = -lgcc -L${PATH_GCC} -L${PATH_NEWLIB} -lc_nano
	endif
endif

all: ${EXEC} ${LIST} ${IMAGE} ${HEXFL}

rebuild: clean all

#
# Gerar executável
#
${EXEC}: ${OBJETOS}
	${LD} -T ${LDSCRIPT} -M=${MAP} ${OBJETOS} -o $@ ${LDOPTS}

#
# Gerar listagem
#
${LIST}: ${EXEC}
	${OBJDMP} -std ${EXEC} > ${LIST}

#
# Gerar imagem binária
#
${IMAGE}: ${EXEC}
	${OBJCPY} -O binary ${EXEC} ${IMAGE}

#
# Gerar arquivo HEX
#
${HEXFL}: ${EXEC}
	${OBJCPY} -O ihex ${EXEC} ${HEXFL}

#
# Compilar arquivos em C
#
.c.o:
	${GCC} ${COPTIONS} -c -o $@ $<

#
# Montar arquivos em assembler
#
.s.o:
	${AS} ${ASMOPTIONS} -o $@ $<

#
# Limpar tudo
#
clean:
	rm -f *.o ${EXEC} ${MAP} ${LIST} ${IMAGE}

#
# Iniciar qemu
#
qemu: ${EXEC}
	@if ps -ax | awk '{print $$5}' | grep '^qemu-system-arm' >/dev/null ; then\
		echo "qemu já está executando"; \
	else qemu-system-arm -s -M raspi2b & \
	fi

#
# Executar openocd
#
ocd:
	@if pgrep openocd >/dev/null ; then \
		echo "openocd já está executando" ; \
	else openocd -f ${OCD_CFG} & \
	fi

#
# Gdb via serial
#
gdb: ${EXEC}
	@if pgrep openocd >/dev/null; then \
		gdb-multiarch ${EXEC} \
			-ex "target extended-remote: 3333" \
			-ex "load"; \
		else gdb-multiarch -b 115200 ${EXEC} \
		                -ex "target remote ${TTY}" \
	                   -ex "load"; \
	fi

#
# Gdb via qemu
#
gdbqemu: ${EXEC}
	gdb -ex "target extended-remote :1234" \
					  -ex "set architecture arm" \
					  -ex "load" \
					  ${EXEC}

#
# Elimina os processos
#
kill:
	-killall openocd
	-killall gdb-multiarch


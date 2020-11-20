PROJECT_NAME = nucleo_767_encoder_example

SRCS = encoder_example.cpp
#SRCS += /path/to/more/sources.cpp

# libopencm3 must be installed and compiled first!
LIBOPENCM3_DIR=../../libopencm3

INCLUDE = ${addprefix -I,${LIBOPENCM3_DIR}/include}

CXX=arm-none-eabi-g++
CXXFLAGS=-g3 -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 \
    --specs=rdimon.specs -DSTM32F7 -nostartfiles \
    -std=c++11
LDFLAGS=-L ${LIBOPENCM3_DIR}/lib -Wl,-T nucleo.ld -lopencm3_stm32f7
#LDFLAGS=-L $(LIBOPENCM3_DIR)/lib -T nucleo.ld -lopencm3_stm32f7

BMP_PORT = /dev/ttyBmpGdb

GDB = arm-none-eabi-gdb
OBJCOPY = arm-none-eabi-objcopy


all: $(PROJECT_NAME).elf

$(PROJECT_NAME).elf: ${SRCS}
#	${CXX} ${CXXFLAGS} $< ${LDFLAGS} -o $@
	${CXX} ${CXXFLAGS} ${INCLUDE} $^ ${LDFLAGS} -o $@
	${OBJCOPY} -O ihex ${PROJECT_NAME}.elf   ${PROJECT_NAME}.hex
	${OBJCOPY} -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin

#st-flash:
#	st-flash write $(PROJ_NAME).bin 0x8000000

bmp-flash:
	${GDB} ${PROJECT_NAME}.elf -nx --batch \
        -ex 'target extended-remote ${BMP_PORT}' \
        -x ./black_magic_probe_flash.scr \
        ${PROJECT_NAME}.elf

clean:
	rm -f *.o *.elf *.hex *.bin

# Put your stlink folder here so make burn will work.
STLINK=../../stlink

# Binaries will be generated with this name (.elf, .bin, .hex, etc)
PROJ_NAME=radio

# Put your STM32F4 library code directory here
STM_COMMON=../../../STM32F407

# Put your source files here (or *.c, etc)
SRCS  = main.c system_stm32f4xx.c
SRCS += delay/delay.c
SRCS += usart/usart.c usart/misc.c
SRCS += i2c/i2c.c
SRCS += rds/rds.c
SRCS += si4703/si4703.c
SRCS += spi/spi1.c
SRCS += st7735/st7735.c st7735/fonts.c
SRCS += gui/gui.c
SRCS += rotary/rotary.c
SRCS += rtc/rtc.c

# Normally you shouldn't need to change anything below this line!
#######################################################################################
GNUGCC = $(STM_COMMON)/gcc-arm-none-eabi/bin
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

CFLAGS  = -g -O0 -Wall -Wno-misleading-indentation -Wno-char-subscripts -Tstm32_flash.ld
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork
# important flag is -fsingle-precision-constant which prevents the double precision emulation
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16 -fsingle-precision-constant
CFLAGS += -I.

# Include files from STM libraries
#CFLAGS += -I$(STM_COMMON)/Utilities/STM32F4-Discovery
CFLAGS += -I$(STM_COMMON)/Libraries/CMSIS/Include
CFLAGS += -I$(STM_COMMON)/Libraries/CMSIS/ST/STM32F4xx/Include
CFLAGS += -I$(STM_COMMON)/Libraries/STM32F4xx_StdPeriph_Driver/inc
CFLAGS += -Idelay
CFLAGS += -Iusart
CFLAGS += -Ii2c
CFLAGS += -Irds
CFLAGS += -Isi4703
CFLAGS += -Ispi
CFLAGS += -Ist7735
CFLAGS += -Igui
CFLAGS += -Irotary
CFLAGS += -Irtc

# add startup file to build
SRCS += $(STM_COMMON)/Libraries/CMSIS/ST/STM32F4xx/Source/Templates/TrueSTUDIO/startup_stm32f4xx.s
OBJS = $(SRCS:.c=.o)

.PHONY: proj

all: $(PROJ_NAME).elf

$(PROJ_NAME).elf: $(SRCS)
	$(CC) $(CFLAGS) $^ -lm -lc -lnosys -o $@
	$(CC) $(CFLAGS) -S $< $^
	$(OBJCOPY) -O ihex $(PROJ_NAME).elf $(PROJ_NAME).hex
	$(OBJCOPY) -O binary $(PROJ_NAME).elf $(PROJ_NAME).bin
	$(SIZE) -B  $(PROJ_NAME).elf
	ls -l $(PROJ_NAME).bin

clean:
	rm -rf *.o $(PROJ_NAME).elf $(PROJ_NAME).hex $(PROJ_NAME).bin *.s
	ls

# Flash the STM32F4
upload: proj
	st-flash write $(PROJ_NAME).bin 0x8000000

OPENOCD	:= openocd -f interface/stlink-v2.cfg -c "transport select hla_swd" -f target/stm32f4x.cfg

test-com:
	$(OPENOCD) -c "init"

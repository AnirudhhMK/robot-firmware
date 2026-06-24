CFLAGS = -mcpu=cortex-m0plus -mthumb -O3 -Iinclude 
LFLAGS := 

SRC_C := $(wildcard src/*.c lib/*.c boot/*.c src/core0/*.c src/core1/*.c) 
SRC_S := $(wildcard boot/*.S lib/*.S)
OBJ := $(SRC_C:%.c=build/%.o) $(SRC_S:%.S=build/%.o)
PICOTOOL := ../picotool/picotool
all: bin/main.elf

build/%.o : %.c
	@mkdir -p $(dir $@)
	arm-none-eabi-gcc -c $(CFLAGS) $< -o $@

build/%.o : %.S
	@mkdir -p $(dir $@)
	arm-none-eabi-gcc -c $(CFLAGS) $< -o $@


bin/main.elf:$(OBJ) boot/script.ld makefile
	@mkdir -p $(dir $@)
	arm-none-eabi-ld $(LFLAGS) -T boot/script.ld $(OBJ) -o bin/main.elf

bin/main.uf2:bin/main.elf
	$(PICOTOOL) uf2 convert bin/main.elf bin/main.uf2

.PHONY : flash
flash:bin/main.uf2
	sudo $(PICOTOOL) load bin/main.uf2

.PHONY : clean
clean:
	rm -rf *.o *.elf *.uf2 




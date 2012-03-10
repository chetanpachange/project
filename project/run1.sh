arm-none-eabi-gcc -g -mcpu=cortex-m3 -mthumb -c main.c vector.c
arm-none-eabi-ld -T linker.ld main.o vector.o -o main.elf
arm-none-eabi-objcopy -O binary main.elf main.bin


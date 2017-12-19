################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/Salathe/Src/GameOfLife.c \
../Drivers/Salathe/Src/but.c \
../Drivers/Salathe/Src/enc.c \
../Drivers/Salathe/Src/font.c \
../Drivers/Salathe/Src/leds.c \
../Drivers/Salathe/Src/oled.c \
../Drivers/Salathe/Src/term.c 

OBJS += \
./Drivers/Salathe/Src/GameOfLife.o \
./Drivers/Salathe/Src/but.o \
./Drivers/Salathe/Src/enc.o \
./Drivers/Salathe/Src/font.o \
./Drivers/Salathe/Src/leds.o \
./Drivers/Salathe/Src/oled.o \
./Drivers/Salathe/Src/term.o 

C_DEPS += \
./Drivers/Salathe/Src/GameOfLife.d \
./Drivers/Salathe/Src/but.d \
./Drivers/Salathe/Src/enc.d \
./Drivers/Salathe/Src/font.d \
./Drivers/Salathe/Src/leds.d \
./Drivers/Salathe/Src/oled.d \
./Drivers/Salathe/Src/term.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/Salathe/Src/%.o: ../Drivers/Salathe/Src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo %cd%
	arm-none-eabi-gcc -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16 -D__weak=__attribute__((weak)) -D__packed=__attribute__((__packed__)) -DUSE_HAL_DRIVER -DSTM32F765xx -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Inc" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Drivers/Salathe" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Drivers/Salathe/Inc" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Drivers/Salathe/Src" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Drivers/STM32F7xx_HAL_Driver/Inc" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Drivers/STM32F7xx_HAL_Driver/Inc/Legacy" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Drivers/CMSIS/Device/ST/STM32F7xx/Include" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Middlewares/Third_Party/FatFs/src" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Middlewares/Third_Party/FreeRTOS/Source/include" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Drivers/CMSIS/Include" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Inc"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



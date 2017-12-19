################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c 

OBJS += \
./Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.o 

C_DEPS += \
./Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/%.o: ../Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo %cd%
	arm-none-eabi-gcc -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16 -D__weak=__attribute__((weak)) -D__packed=__attribute__((__packed__)) -DUSE_HAL_DRIVER -DSTM32F765xx -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Inc" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Drivers/Salathe" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Drivers/Salathe/Inc" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Drivers/Salathe/Src" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Drivers/STM32F7xx_HAL_Driver/Inc" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Drivers/STM32F7xx_HAL_Driver/Inc/Legacy" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Drivers/CMSIS/Device/ST/STM32F7xx/Include" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Middlewares/Third_Party/FatFs/src" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Middlewares/Third_Party/FreeRTOS/Source/include" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Drivers/CMSIS/Include" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Inc"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



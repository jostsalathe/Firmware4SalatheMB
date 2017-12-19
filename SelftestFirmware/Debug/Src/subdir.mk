################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/adc.c \
../Src/bsp_driver_sd.c \
../Src/fatfs.c \
../Src/fatfs_platform.c \
../Src/fmc.c \
../Src/freertos.c \
../Src/gpio.c \
../Src/main.c \
../Src/rng.c \
../Src/rtc.c \
../Src/sai.c \
../Src/sd_diskio.c \
../Src/sdmmc.c \
../Src/spi.c \
../Src/stm32f7xx_hal_msp.c \
../Src/stm32f7xx_hal_timebase_TIM.c \
../Src/stm32f7xx_it.c \
../Src/system_stm32f7xx.c \
../Src/tim.c \
../Src/usart.c \
../Src/usb_otg.c 

OBJS += \
./Src/adc.o \
./Src/bsp_driver_sd.o \
./Src/fatfs.o \
./Src/fatfs_platform.o \
./Src/fmc.o \
./Src/freertos.o \
./Src/gpio.o \
./Src/main.o \
./Src/rng.o \
./Src/rtc.o \
./Src/sai.o \
./Src/sd_diskio.o \
./Src/sdmmc.o \
./Src/spi.o \
./Src/stm32f7xx_hal_msp.o \
./Src/stm32f7xx_hal_timebase_TIM.o \
./Src/stm32f7xx_it.o \
./Src/system_stm32f7xx.o \
./Src/tim.o \
./Src/usart.o \
./Src/usb_otg.o 

C_DEPS += \
./Src/adc.d \
./Src/bsp_driver_sd.d \
./Src/fatfs.d \
./Src/fatfs_platform.d \
./Src/fmc.d \
./Src/freertos.d \
./Src/gpio.d \
./Src/main.d \
./Src/rng.d \
./Src/rtc.d \
./Src/sai.d \
./Src/sd_diskio.d \
./Src/sdmmc.d \
./Src/spi.d \
./Src/stm32f7xx_hal_msp.d \
./Src/stm32f7xx_hal_timebase_TIM.d \
./Src/stm32f7xx_it.d \
./Src/system_stm32f7xx.d \
./Src/tim.d \
./Src/usart.d \
./Src/usb_otg.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o: ../Src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo %cd%
	arm-none-eabi-gcc -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16 -D__weak=__attribute__((weak)) -D__packed=__attribute__((__packed__)) -DUSE_HAL_DRIVER -DSTM32F765xx -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Inc" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Drivers/Salathe" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Drivers/Salathe/Inc" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Drivers/Salathe/Src" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Drivers/STM32F7xx_HAL_Driver/Inc" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Drivers/STM32F7xx_HAL_Driver/Inc/Legacy" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Drivers/CMSIS/Device/ST/STM32F7xx/Include" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Middlewares/Third_Party/FatFs/src" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Middlewares/Third_Party/FreeRTOS/Source/include" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Drivers/CMSIS/Include" -I"D:/Eigene Daten/STM32-Projekte/SelftestFirmware/Inc"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



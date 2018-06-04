################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/algoritmosDistribucion/algoritmosDistribucion.c 

OBJS += \
./src/algoritmosDistribucion/algoritmosDistribucion.o 

C_DEPS += \
./src/algoritmosDistribucion/algoritmosDistribucion.d 


# Each subdirectory must supply rules for building sources it contributes
src/algoritmosDistribucion/%.o: ../src/algoritmosDistribucion/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2018-1c-Los-Simuladores/libraries" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



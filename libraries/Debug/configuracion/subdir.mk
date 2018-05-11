################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../configuracion/configuracion.c 

OBJS += \
./configuracion/configuracion.o 

C_DEPS += \
./configuracion/configuracion.d 


# Each subdirectory must supply rules for building sources it contributes
configuracion/%.o: ../configuracion/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



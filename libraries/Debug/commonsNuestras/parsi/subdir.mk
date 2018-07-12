################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../commonsNuestras/parsi/parser.c 

OBJS += \
./commonsNuestras/parsi/parser.o 

C_DEPS += \
./commonsNuestras/parsi/parser.d 


# Each subdirectory must supply rules for building sources it contributes
commonsNuestras/parsi/%.o: ../commonsNuestras/parsi/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



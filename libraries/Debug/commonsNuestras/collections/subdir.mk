################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../commonsNuestras/collections/dictionary.c \
../commonsNuestras/collections/list.c \
../commonsNuestras/collections/queue.c 

OBJS += \
./commonsNuestras/collections/dictionary.o \
./commonsNuestras/collections/list.o \
./commonsNuestras/collections/queue.o 

C_DEPS += \
./commonsNuestras/collections/dictionary.d \
./commonsNuestras/collections/list.d \
./commonsNuestras/collections/queue.d 


# Each subdirectory must supply rules for building sources it contributes
commonsNuestras/collections/%.o: ../commonsNuestras/collections/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



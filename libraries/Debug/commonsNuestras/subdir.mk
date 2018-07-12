################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../commonsNuestras/bitarray.c \
../commonsNuestras/config.c \
../commonsNuestras/error.c \
../commonsNuestras/log.c \
../commonsNuestras/process.c \
../commonsNuestras/string.c \
../commonsNuestras/temporal.c \
../commonsNuestras/txt.c 

OBJS += \
./commonsNuestras/bitarray.o \
./commonsNuestras/config.o \
./commonsNuestras/error.o \
./commonsNuestras/log.o \
./commonsNuestras/process.o \
./commonsNuestras/string.o \
./commonsNuestras/temporal.o \
./commonsNuestras/txt.o 

C_DEPS += \
./commonsNuestras/bitarray.d \
./commonsNuestras/config.d \
./commonsNuestras/error.d \
./commonsNuestras/log.d \
./commonsNuestras/process.d \
./commonsNuestras/string.d \
./commonsNuestras/temporal.d \
./commonsNuestras/txt.d 


# Each subdirectory must supply rules for building sources it contributes
commonsNuestras/%.o: ../commonsNuestras/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables
CPP_SRCS += \
../src/config.cpp \
../src/gweled_hack.cpp \
../src/main.cpp \
../src/process_memory.cpp

OBJS += \
./src/config.o \
./src/gweled_hack.o \
./src/main.o \
./src/process_memory.o

CPP_DEPS += \
./src/config.d \
./src/gweled_hack.d \
./src/main.d \
./src/process_memory.d


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



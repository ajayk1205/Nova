################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Config.cpp \
../src/FeatureSet.cpp \
../src/Logger.cpp \
../src/NovaUtil.cpp \
../src/Point.cpp \
../src/SerializationHelper.cpp \
../src/Suspect.cpp \
../src/WhitelistConfiguration.cpp 

OBJS += \
./src/Config.o \
./src/FeatureSet.o \
./src/Logger.o \
./src/NovaUtil.o \
./src/Point.o \
./src/SerializationHelper.o \
./src/Suspect.o \
./src/WhitelistConfiguration.o 

CPP_DEPS += \
./src/Config.d \
./src/FeatureSet.d \
./src/Logger.d \
./src/NovaUtil.d \
./src/Point.d \
./src/SerializationHelper.d \
./src/Suspect.d \
./src/WhitelistConfiguration.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0  `pkg-config --libs --cflags libnotify` -std=c++0x -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



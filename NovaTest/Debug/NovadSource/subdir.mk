################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../NovadSource/ClassificationAggregator.cpp \
../NovadSource/ClassificationEngine.cpp \
../NovadSource/ClassificationEngineFactory.cpp \
../NovadSource/Control.cpp \
../NovadSource/KnnClassification.cpp \
../NovadSource/Novad.cpp \
../NovadSource/NovadCallback.cpp \
../NovadSource/ProtocolHandler.cpp \
../NovadSource/Threads.cpp \
../NovadSource/ThresholdTriggerClassification.cpp 

OBJS += \
./NovadSource/ClassificationAggregator.o \
./NovadSource/ClassificationEngine.o \
./NovadSource/ClassificationEngineFactory.o \
./NovadSource/Control.o \
./NovadSource/KnnClassification.o \
./NovadSource/Novad.o \
./NovadSource/NovadCallback.o \
./NovadSource/ProtocolHandler.o \
./NovadSource/Threads.o \
./NovadSource/ThresholdTriggerClassification.o 

CPP_DEPS += \
./NovadSource/ClassificationAggregator.d \
./NovadSource/ClassificationEngine.d \
./NovadSource/ClassificationEngineFactory.d \
./NovadSource/Control.d \
./NovadSource/KnnClassification.d \
./NovadSource/Novad.d \
./NovadSource/NovadCallback.d \
./NovadSource/ProtocolHandler.d \
./NovadSource/Threads.d \
./NovadSource/ThresholdTriggerClassification.d 


# Each subdirectory must supply rules for building sources it contributes
NovadSource/%.o: ../NovadSource/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I../../NovaLibrary/src -I../../Novad/src -I../../Nova_UI_Core/src -O0 -g3 -Wall -c -fmessage-length=0 -pthread -std=c++0x -fprofile-arcs -ftest-coverage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



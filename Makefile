CXX      = arm-none-eabi-g++
CPU      = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
CXXFLAGS = $(CPU) -std=c++17 -O0 -g3 -ffreestanding -nostdlib \
            -fno-exceptions -fno-rtti \
            -Wall -Wextra \
            -I.

LDFLAGS  = $(CPU) -nostdlib -T linker.ld -lgcc

SRCS = main.cpp \
       src/startup/startup.cpp \
       src/hal/uart.cpp \
       src/hal/gpio.cpp \
       src/hal/spi.cpp \

BIN_DIR = bin
TARGET  = $(BIN_DIR)/firmware.elf

OBJS = $(SRCS:%.cpp=$(BIN_DIR)/%.o)

all: $(TARGET)

$(BIN_DIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) -o $(TARGET)

clean:
	rm -rf $(BIN_DIR)

.PHONY: all clean
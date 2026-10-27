CXX     = arm-none-eabi-g++
CPU     = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
CXXFLAGS = $(CPU) -std=c++17 -O0 -g3 -ffreestanding -nostdlib \
            -fno-exceptions -fno-rtti \
            -Wall -Wextra -Wconversion \
            -I.

LDFLAGS  = $(CPU) -nostdlib -T linker.ld -lgcc

SRCS_CXX = main.cpp \
           src/startup/startup.cpp \
           src/hal/uart.cpp \
           src/hal/gpio.cpp \
           src/hal/spi.cpp \
           src/rte_core/task.cpp \
           src/rte_core/scheduler.cpp \
           src/rte_core/isr_handlers.cpp \
           src/drivers/icm42688.cpp

SRCS_ASM = src/rte_core/context_switch.s

BIN_DIR  = bin
TARGET   = $(BIN_DIR)/firmware.elf

OBJS_CXX = $(SRCS_CXX:%.cpp=$(BIN_DIR)/%.o)
OBJS_ASM = $(SRCS_ASM:%.s=$(BIN_DIR)/%.o)
OBJS     = $(OBJS_CXX) $(OBJS_ASM)

.PHONY: all clean

all: $(TARGET)

$(BIN_DIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR)/%.o: %.s
	mkdir -p $(dir $@)
	$(CXX) $(CPU) -c $< -o $@

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) -o $(TARGET)

clean:
	rm -rf $(BIN_DIR)


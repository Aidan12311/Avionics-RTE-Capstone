CXX     = arm-none-eabi-g++
CPU     = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
CXXFLAGS = $(CPU) -std=c++17 -O0 -g3 -ffreestanding -nostdlib \
            -fno-exceptions -fno-rtti
LDFLAGS  = $(CPU) -nostdlib -T linker.ld -lgcc

SRCS = main.cpp src/startup/startup.cpp
OBJS = $(SRCS:.cpp=.o)

BIN_DIR = bin
TARGET  = $(BIN_DIR)/firmware.elf

all: $(BIN_DIR) $(TARGET)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) -o $(TARGET)

clean:
	find . -name "*.o" -delete
	rm -rf $(BIN_DIR)

.PHONY: all clean

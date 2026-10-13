#include "src/hal/uart.hpp"

int main() {
    uart::init();
    
    uart::println("\n\rhello world");
    uart::print("!");
    
    uart::put_u32(15);
    uart::put_char('\n');

    uart::put_hex(0xFF);

    while(1);
}

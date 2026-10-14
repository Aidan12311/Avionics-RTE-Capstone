#ifndef AVIONICS_RTE_GPIO_HPP
#define AVIONICS_RTE_GPIO_HPP

#include <cstdint>

namespace gpio {
    enum class Port { A, B, C, D, E, F, G, H };

    class Pin {
    public:
        constexpr Pin(Port port, uint8_t number) 
        : _port(port), _number(number) {}
    
        void configure_output() const;

        void configure_af(uint8_t af) const;

        void set_high() const;
        void set_low() const;
        void toggle() const;

    private:
        Port _port{};
        uint8_t _number{};

        volatile uint32_t *base() const;
    };
}

#endif // AVIONICS_RTE_GPIO_HPP

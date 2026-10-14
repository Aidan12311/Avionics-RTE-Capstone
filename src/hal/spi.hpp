#ifndef AVIONICS_RTE_SPI_HPP
#define AVIONICS_RTE_SPI_HPP

#include <cstdint>

namespace spi {
    void init();

    bool is_busy();

    uint8_t transfer(uint8_t data);
} // namespace spi

#endif // AVIONICS_RTE_SPI_HPP

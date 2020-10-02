#ifndef SPI_H
#define SPI_H

#include <cstdint>
#include <vector>
#include <string_view>
#include <cstring>
#include <iostream>

class SPI {

    void pabort(const char *s);

    const char* m_device {"/dev/spidev0.0"};
    uint8_t m_mode {0};
    uint8_t m_bits {8};
    uint32_t m_speed {1000000};
    uint16_t m_delay {0};

    int m_fd {-1};

public:

    SPI();
    ~SPI();

    std::vector<uint8_t> transfer(std::vector<uint8_t>&& input);


};

#endif

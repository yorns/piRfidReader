#include "spi.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

void SPI::pabort(const char *s)
{
    perror(s);
    abort();
}

SPI::SPI() {

    int ret = 0;

    //
    // Parse mode of SPI device, option 0, 1, 2, 3
    //
    uint8_t spi_mode;
    switch(m_mode) {
    case 0:
        spi_mode = SPI_MODE_0;
        break;
    case 1:
        spi_mode = SPI_MODE_1;
        break;
    case 2:
        spi_mode = SPI_MODE_2;
        break;
    case 3:
        spi_mode = SPI_MODE_3;
        break;
    default:
        spi_mode = SPI_MODE_0;
    }

    std::cout << "opening device <"<< m_device << "> with\n";
    std::cout << "mode: " << m_mode << "\n";
    std::cout << "bits: " << m_bits << "\n";
    std::cout << "speed: " << m_speed << "\n";
    std::cout << "delay: " << m_delay << "\n";

    m_fd = open(m_device, O_RDWR);
    if (m_fd < 0) {
        pabort("can't open device");
    }

    ret = ioctl(m_fd, SPI_IOC_WR_MODE, &spi_mode);
    if (ret == -1) {
        pabort("can't set spi mode");
    }

    ret = ioctl(m_fd, SPI_IOC_RD_MODE, &spi_mode);
    if (ret == -1) {
        pabort("can't get spi mode");
    }


    ret = ioctl(m_fd, SPI_IOC_WR_BITS_PER_WORD, &m_bits);
    if (ret == -1)
        pabort("SPI_IOC_WR_BITS_PER_WORD failed");

    ret = ioctl(m_fd, SPI_IOC_RD_BITS_PER_WORD, &m_bits);
    if (ret == -1)
        pabort("SPI_IOC_RD_BITS_PER_WORD failed");

    ret = ioctl(m_fd, SPI_IOC_WR_MAX_SPEED_HZ, &m_speed);
    if (ret == -1)
        pabort("SPI_IOC_WR_MAX_SPEED_HZ failed");

    ret = ioctl(m_fd, SPI_IOC_RD_MAX_SPEED_HZ, &m_speed);
    if (ret == -1)
        pabort("SPI_IOC_RD_MAX_SPEED_HZ failed");

    std::cout << "spi connection opened successful\n";
}

SPI::~SPI() {
    close(m_fd);
}

std::vector<uint8_t> SPI::transfer(std::vector<uint8_t> &&input) {

    std::vector<uint8_t> output(input.size(), '\0');

    spi_ioc_transfer tr;
    memset(&tr, 0, sizeof(tr));
    tr.tx_buf = (uint64_t)(input.data()); // tr want a point in form of a uint64 integer
    tr.rx_buf = (uint64_t)(output.data()); // tr want a point in form of a uint64 integer
    tr.len = static_cast<uint32_t>(input.size());
    tr.delay_usecs = m_delay;
    tr.speed_hz = m_speed;
    tr.bits_per_word = m_bits;
    tr.cs_change = 0;

    int ret (-1);
    ret = ioctl(m_fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 0) {
        pabort("can't send spi message ");
    }

    return output;
}

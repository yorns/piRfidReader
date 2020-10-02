#ifndef RFID_H
#define RFID_H

#include <iostream>
#include <cstdint>
#include <unistd.h>
#include <tuple>
#include "spi.h"
#include "PCD_Command.h"
#include "PICC_Command.h"
#include "Register.h"

#define MAX_LEN 16

enum Status {
    MI_OK = 1,
    MI_ERR = 3,
    MI_NOTAGERR = 4
};

class Rfid {

    SPI& m_spi;
public:

    Rfid(SPI& spi);

    void init();

    void writeToRegister(uint8_t address, uint8_t value);
    void writeToRegister(uint8_t address, std::vector<uint8_t>&& data);

    std::vector<uint8_t> readRegister(uint8_t address, uint8_t size = 1);

    void clearBitmask(uint8_t reg, uint8_t mask);
    void setBitmask(uint8_t reg, uint8_t mask);

    void setAntennaOn();

    std::tuple<Status, std::vector<uint8_t>, uint32_t> toCard(uint8_t command, std::vector<uint8_t> sendData);

    std::tuple<Status, uint32_t> request(uint8_t reqMode);

    std::tuple<Status,std::vector<uint8_t>> anticoll();

    uint8_t selectTag(std::vector<uint8_t> serNum);

    std::vector<uint8_t> calulateCRC(const std::vector<uint8_t>& pIndata);

    Status auth(uint8_t authMode, uint8_t BlockAddr,
                const std::vector<uint8_t>& Sectorkey,
                const std::vector<uint8_t>& serNum);

    void read(uint8_t blockAddr);

    void stopCrypto1();


};

#endif // RFID_H

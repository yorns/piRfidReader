#include "rfid.h"

Rfid::Rfid(SPI &spi): m_spi(spi) {}

void Rfid::writeToRegister(uint8_t address, uint8_t value) {

    uint8_t writeAddress(address<<1 & 0x7e);

    m_spi.transfer({writeAddress, value});

}

void Rfid::writeToRegister(uint8_t address, std::vector<uint8_t> &&data) {

    //std::vector<uint8_t> _data;
    uint8_t writeAddress(address<<1 & 0x7e);
    for (auto val : data) {
        m_spi.transfer({writeAddress, val});
    }
}

std::vector<uint8_t> Rfid::readRegister(uint8_t address, uint8_t size) {

    std::vector<uint8_t> outdata(size, '\0');

    uint8_t readAddress ( ((address << 0x01) & 0x7e) | 0x80 );

    // do we really need to do that this way?
    auto gupfl = m_spi.transfer({readAddress}); // ignore output
    //std::cerr << "read: 0x"<<std::hex << (int)gupfl.at(0)<<"\n";

    if (size == 1) {
        outdata[0] = m_spi.transfer({0}).at(0);
        return outdata;
    }

    for(uint32_t i{0}; i<size-1; ++i)
    {
        //std::cerr << ".";
        outdata[i] = m_spi.transfer({readAddress}).at(0);
    }
    //std::cerr << "\n";

    outdata[size-1] = m_spi.transfer({0}).at(0);

    return outdata;
}

void Rfid::clearBitmask(uint8_t reg, uint8_t mask) {
    auto tmp = readRegister(reg);
    uint8_t bitSet = tmp.at(0) & (~mask);
    writeToRegister(reg, bitSet);

}

void Rfid::setBitmask(uint8_t reg, uint8_t mask) {
    auto tmp = readRegister(reg);
    uint8_t bitSet = tmp.at(0) | mask;
    writeToRegister(reg, bitSet);
}

void Rfid::setAntennaOn() {

    auto antennaBytes = readRegister(Register::TxControlReg);
    if ((antennaBytes.at(0) & 0x03) != 0x03) {
        uint8_t value = antennaBytes.at(0) | 0x03;
        writeToRegister(Register::TxControlReg, value);
        {
            auto antennaBytes = readRegister(Register::TxControlReg);
        }
    }

}

std::tuple<Status, std::vector<uint8_t>, uint32_t> Rfid::toCard(uint8_t command, std::vector<uint8_t> sendData)
{
    Status status = Status::MI_ERR;
    uint8_t irqEn = 0x00;
    uint8_t waitIRq = 0x00;

    uint32_t backLengthBit{0};
    std::vector<uint8_t> backData;

    if (command == PCD_Command::MFAuthent) {
        irqEn = 0x12;
        waitIRq = 0x10;
    }

    if (command == PCD_Command::Transceive) {
        irqEn = 0x77;
        waitIRq = 0x30;
    }

    writeToRegister(Register::ComIEnReg, irqEn|0x80);
    clearBitmask(Register::ComIrqReg, 0x80);
    setBitmask(Register::FIFOLevelReg, 0x80);

    writeToRegister(Register::CommandReg, PCD_Command::Idle);

    for(const auto& val : sendData)
        writeToRegister(Register::FIFODataReg, val);

    writeToRegister(Register::CommandReg, command);

    if (command == PCD_Command::Transceive)
        setBitmask(Register::BitFramingReg, 0x80);

    uint8_t val{0};
    uint32_t i{2000};
    for (; i>0; --i) {
        auto n = readRegister(Register::ComIrqReg);
        val = n.at(0);
        if ((val&0x01) || (val&waitIRq))
            break;
    }

    clearBitmask(Register::BitFramingReg, 0x80);

    if (i) {
        //        std::cout << "\n#################### irq register set, command completed #####################################\n\n";
        if ((readRegister(Register::ErrorReg).at(0) & 0x1B)==0x00) {
            status = Status::MI_OK;

            if (val & irqEn & 0x01)
                status = Status::MI_NOTAGERR;

            if (command == PCD_Command::Transceive) {
                auto n = readRegister(Register::FIFOLevelReg).at(0);
                //std::cout << "received <"<<std::dec << (int)n << "> datablocks\n";
                auto lastBits = readRegister(Register::ControlReg).at(0) & 0x07;

                if (lastBits != 0) {
                    backLengthBit = (n-1)*8;
                    backLengthBit += static_cast<uint32_t>(lastBits);
                }
                else {
                    backLengthBit = n*8;
                }

                if (n == 0)n = 1;
                else if (n > MAX_LEN) n = MAX_LEN;

                for(uint32_t i{0}; i<n; ++i)
                    backData.push_back(readRegister(Register::FIFODataReg).at(0));

                return std::make_tuple(status,backData,backLengthBit);

            }
        }
    }
    else {
        std::cout << "irq timeout\n";
    }


    return std::make_tuple(status, std::vector<uint8_t> {}, 0);

}

std::tuple<Status, uint32_t> Rfid::request(uint8_t reqMode)
{

    writeToRegister(Register::BitFramingReg, 0x07);

    std::vector<uint8_t> TagType;
    TagType.emplace_back(reqMode);
    auto [status,backData,backBits] = toCard(PCD_Command::Transceive, TagType);

            if ((status != Status::MI_OK) | (backBits != 0x10)) {
        status = Status::MI_ERR;
    } else {
    }

    return std::make_tuple(status,backBits);

}

std::tuple<Status, std::vector<uint8_t> > Rfid::anticoll() {
    //backData = []
    uint8_t serNumCheck { 0 };

    std::vector<uint8_t> serNum;

    writeToRegister(Register::BitFramingReg, 0x00);

    serNum.emplace_back(PICC_Command::PICC_CMD_ANTICOL);
    serNum.emplace_back(0x20);

    auto [status,backData,backBits] = toCard(PCD_Command::Transceive, serNum);

            if(status == Status::MI_OK) {
        if (backData.size() == 5) {
            for(uint32_t i{0}; i<4; ++i)
                serNumCheck = serNumCheck ^ backData[i];
            if (serNumCheck != backData[4])
                status = Status::MI_ERR;
        }
        else {
            status = Status::MI_ERR;
        }
    }

    return std::make_tuple(status,backData);
}

uint8_t Rfid::selectTag(std::vector<uint8_t> serNum) {

    std::vector<uint8_t> buf;

    buf.emplace_back(PICC_Command::PICC_CMD_SEL_CL1);
    buf.emplace_back(0x70);

    buf.insert(buf.end(), std::begin(serNum), std::end(serNum));

    auto pOut = calulateCRC(buf);

    buf.insert(buf.end(), std::begin(pOut), std::end(pOut));

    auto [status, backData, backLen] = toCard(PCD_Command::Transceive, buf);

            if ((status == Status::MI_OK) && (backLen == 0x18)) {
        return backData[0];
    }

    return 0;
}

std::vector<uint8_t> Rfid::calulateCRC(const std::vector<uint8_t> &pIndata) {

    clearBitmask(Register::DivIrqReg, 0x04);
    setBitmask(Register::FIFOLevelReg, 0x80);

    for (const auto& val : pIndata)
        writeToRegister(Register::FIFODataReg, val);

    writeToRegister(Register::CommandReg, PCD_Command::CalcCRC);

    for(uint32_t i{0}; i<0xff; ++i) {
        auto list = readRegister(Register::DivIrqReg);
        if (list.at(0) & 0x04)
            break;
    }

    std::vector<uint8_t> pOutData;
    pOutData.emplace_back(readRegister(Register::CRCResultRegL).at(0));
    pOutData.emplace_back(readRegister(Register::CRCResultRegH).at(0));
    return pOutData;
}

Status Rfid::auth(uint8_t authMode, uint8_t BlockAddr, const std::vector<uint8_t> &Sectorkey, const std::vector<uint8_t> &serNum)
{
    std::vector<uint8_t> buff;

    // First byte should be the authMode (A or B)
    buff.emplace_back(authMode);

    // Second byte is the trailerBlock (usually 7)
    buff.emplace_back(BlockAddr);

    // Now we need to append the authKey which usually is 6 bytes of 0xFF
    buff.insert(buff.end(), std::begin(Sectorkey), std::end(Sectorkey));

    // Next we append the first 4 bytes of the UID
    for(uint32_t i{0}; i<4; ++i)
        buff.push_back(serNum[i]);

    // Now we start the authentication itself
    auto [status, backData, backLen] = toCard(PCD_Command::MFAuthent, buff);


            // Check if an error occurred
            if (status != Status::MI_OK)
            std::cout << "AUTH ERROR!!\n";

            if ((readRegister(Register::Status2Reg).at(0) & 0x08) == 0)
            std::cout << "AUTH ERROR(status2reg & 0x08) != 0\n";

            // Return the status
            return status;
}

            void Rfid::read(uint8_t blockAddr) {
        std::vector<uint8_t> recvData;
        recvData.push_back(PICC_Command::PICC_CMD_MF_READ);
        recvData.push_back(blockAddr);
        auto pOut = calulateCRC(recvData);
        recvData.push_back(pOut[0]);
        recvData.push_back(pOut[1]);

        auto [status, backData, backLen] = toCard(PCD_Command::Transceive, recvData);

                if (status != Status::MI_OK) {
            std::cout << "Error while reading!\n";
        }

        //        if (backData.size() == 16) {
        //            std::cout << "Sector " << std::hex<< (int)blockAddr << "[";
        //            for(const auto val : backData)
        //                std::cout << " 0x"<<std::hex << (int) val;
        //            std::cout << "]\n";
        //        }
    }

    void Rfid::stopCrypto1() {
        clearBitmask(Register::Status2Reg, 0x08);
    }

    void Rfid::init() {

        writeToRegister(Register::CommandReg, PCD_Command::SoftReset);

        while ( (readRegister(Register::CommandReg).at(0) & 0x10) != 0x00 ) {
            std::cout << "Wait for the PowerDown bit in CommandReg to be cleared (0x"<<std::hex << (int)readRegister(Register::CommandReg).at(0) <<")\n";
            //            std::cerr <<".";
            usleep(10000);
            //            // PCD still restarting - unlikely after waiting 50ms, but better safe than sorry.
        }
        std::cout << "Soft Reset done\n";
        std::cout << "CommandReg value is 0x"<<std::hex << (int) readRegister(Register::CommandReg).at(0) <<"\n";



        std::cout << "Setting up SPI registers for RFID\n";

        writeToRegister(Register::TModeReg, 0x8D);
        writeToRegister(Register::TPrescalerReg, 0x3E);
        writeToRegister(Register::TReloadRegL, 30);
        writeToRegister(Register::TReloadRegH, 0);

        writeToRegister(Register::TxASKReg, 0x40);
        writeToRegister(Register::ModeReg, 0x3D);

        setAntennaOn();
        std::cout << "SPI: register done\n";


    }

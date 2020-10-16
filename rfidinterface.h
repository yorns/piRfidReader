#ifndef RFIDINTERFACE_H
#define RFIDINTERFACE_H

#include <chrono>
#include "repeattimer.h"
#include "rfid.h"
#include "spi.h"
#include "Debounce.h"

using namespace std::chrono_literals;

class RfidInterface {
    SPI spiDevice;
    Rfid rfid;
    Debounce key;

    RepeatTimer repeatTimer;

    void rfidCaller();


public:
    RfidInterface(boost::asio::io_context& context);

    void setKeyHandler(std::function<void(std::vector<uint8_t>)>&& keyHandler);
};

#endif // RFIDINTERFACE_H

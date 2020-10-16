#include "rfidinterface.h"


void RfidInterface::rfidCaller()
{
    // Scan for cards
    auto [status,TagType] = rfid.request(PICC_Command::PICC_CMD_REQA);

            // If a card is found
            if (status == Status::MI_OK) {

        auto [status1,uid] = rfid.anticoll();

                if (status1 == Status::MI_OK) {
            key.setCardId(uid);
        } else {
            key.setCardId({0,0,0,0,0});
        }

        // somehow rfid is out of sync, no clue how to solve this just a try
        rfid.request(PICC_Command::PICC_CMD_REQA);
    }
    else {
        key.setCardId({0,0,0,0,0});
    }
}

RfidInterface::RfidInterface(boost::asio::io_context &context)
    : rfid(spiDevice), repeatTimer(context, 50ms) {
    rfid.init();
    repeatTimer.setHandler([this](){rfidCaller();});
    repeatTimer.start();
}

void RfidInterface::setKeyHandler(std::function<void (std::vector<uint8_t>)> &&keyHandler) {
    key.setNewCardHandler(std::move(keyHandler));
}

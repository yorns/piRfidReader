#include <iostream>
#include <cstdint>
#include <functional>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include "stickentry.h"
#include "rfidinterface.h"

std::optional<uint64_t> generateID(std::vector<uint8_t> cardId)
{
    if (cardId.size() == 5) {

        int shift {0};
        uint64_t id {0};
        uint64_t value {0};
        for (const auto& val : cardId) {
            value = val;
            id += value << shift;
            shift += 8;
        }
        return id;
    }

    return std::nullopt;
}


int main()
{
    boost::asio::io_context ioc;

    // create the rfid interface to the hardware
    RfidInterface rfidInterface(ioc);

    uint64_t connected { 0x00 };

    auto keyHandler = [&connected](std::vector<uint8_t> cardId) {

       if (auto _id = generateID(cardId)) {

            uint64_t id = *_id;
            Action action { Action::release };

            // is this a release or a connect?
            if (id != 0x0) {
                action = Action::connect;
                std::cout << "connect: 0x" << std::hex << id << std::dec << "\n";
                connected = id;
            } else {
                if (connected != 0x00) {
                    action = Action::release; // just to be clear here
                    std::cout << "disconnect: 0x" << std::hex << connected << std::dec << "\n";
                    connected = 0x00;
                }
            }
       }
       else {
           std::cerr << "cannot generate key id\n";
       }

       return;

    };

    rfidInterface.setKeyHandler(keyHandler);

    ioc.run();

    return 0;

}

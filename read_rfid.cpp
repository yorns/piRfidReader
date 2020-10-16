#include <iostream>
#include <cstdint>
#include <functional>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include "stickentry.h"
#include "table.h"
#include "rfidinterface.h"

std::optional<uint64_t> generateID(std::vector<uint8_t> cardId)
{
    std::cout << "New card detected with ID (LSB order) [";
    for (const auto& val : cardId)
        std::cout << " 0x"<<std::hex << static_cast<int>(val);
    std::cout << " ]\n";
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

    auto table = Table();
    table.readStickDatabase();

    auto keyHandler = [&table](std::vector<uint8_t> cardId) {

       if (auto _id = generateID(cardId)) {

            uint64_t id = *_id;
            Action action { Action::release };

            // is this a release or a connect?
            if (id != 0x0) {
                action = Action::connect;
            } else {
                action = Action::release; // just to be clear here

                if (table.hasCurrent()) {
                    id = table.getCurrent().getKeyID();
                }
                else {
                    std::cerr << "Error: release with unknown ID\n";
                    return;
                }

            }

            if (auto stickElem = table.find(id)) {
                std::cout << stickElem->dump();
                return;
            }
            std::cerr << "unknown stick\n";
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

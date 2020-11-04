#include <iostream>
#include <cstdint>
#include <functional>
#include "spi.h"
#include "rfid.h"
#include "Debounce.h"
#include "repeattimer.h"
#include <snc/client.h>
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


int main(int argc, char* argv[])
{
    boost::asio::io_context ioc;

    // create the rfid interface to the hardware
    RfidInterface rfidInterface(ioc);

    // send data through snc
    snc::Client client("cardreader", ioc, "127.0.0.1", 12001);

    std::string tableConfigFile;
    if (argc == 2) {
        tableConfigFile = argv[1];
    }

    auto table = Table(tableConfigFile);
    table.readStickDatabase();

    client.broadcastHandler([&table](const std::string& , const std::string& msg) {
       //std::cout << "broadcast message receive from <"<<nick<<">\n";
        try {
            auto jsonMsg = nlohmann::json::parse(msg);
            auto broadcastMsg = jsonMsg.find("SongBroadcastMessage");
            if (broadcastMsg != jsonMsg.end()) {
                table.updateFromJson(*broadcastMsg);
                table.writeStickEntries(); // persist table
            }
        } catch (std::exception& ex) {
            std::cerr << "exception <"<<ex.what() << ">\n"<<msg<<"\n";
            return;
        }
    });

    auto keyHandler = [&table, &client](std::vector<uint8_t> cardId) {

       if (auto _id = generateID(cardId)) {

            uint64_t id = *_id;
            Action action { Action::release };

            // is this a release or a connect?
            if (id != 0x0) {
                action = Action::connect;
                if (!table.setCurrent(id))
                    return;
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
                auto message = stickElem->generateCmdMsg(action);
                if (action == Action::release)
                    table.unsetCurrent();
                std::cout << "send message: " << message.dump(2) << "\n";
                client.send(snc::Client::SendType::cl_send, "audioserver", message.dump());
                return;
            }
       }
       std::cerr << "unknown stick\n";
    };

    rfidInterface.setKeyHandler(keyHandler);

    ioc.run();

    return 0;

}

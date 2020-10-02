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

using namespace std::chrono_literals;

static std::string configFileName { "/var/audioplayer/stick/stick.conf" };

enum class Action {
    connect,
    release
};

enum class StickFunctionality {
    freeStick,
    boundStick
};

enum class StickNameType {
    isID,
    isName
};

struct StickEntry {
    uint64_t m_id {0};
    std::string m_albumID;
    std::string m_titleID;
    uint32_t m_position {0};
    StickFunctionality m_stickType { StickFunctionality::boundStick };
    StickNameType m_stickNameType { StickNameType::isName };

    StickEntry(uint64_t id, std::string&& album, std::string&& title, uint32_t position, StickFunctionality stick, StickNameType stickNameType)
        : m_id(id), m_albumID(std::move(album)), m_titleID(std::move(title)), m_position(position), m_stickType(stick), m_stickNameType(stickNameType) {}
    StickEntry() = default;

    std::string toJson() { return ""; }
    void fromJson(std::string& jsonData) { }
};

std::vector<StickEntry> readStickEntries()
{

    // read stick config

    std::vector<StickEntry> table {
        {0xffffffffff, "", "", 0, StickFunctionality::boundStick, StickNameType::isID },
        {0xad5f7e0488, "WDR", "WDR2 Ruhrgebiet", 0, StickFunctionality::boundStick, StickNameType::isName },
        {0xd35f000488, "WDR", "Maus", 0, StickFunctionality::boundStick, StickNameType::isName },
        {0xcd5f1e0488, "", "", 0, StickFunctionality::freeStick, StickNameType::isID }
    // new available stick
    };

    // read json


    return table;
}

void writeStickEntries(std::vector<StickEntry>& table) {
    // write
}

int main()
{
    boost::asio::io_context ioc;

    SPI spiDevice;
    Rfid rfid(spiDevice);
    snc::Client client("cardreader", ioc, "127.0.0.1", 12001);

    std::vector<uint8_t> lastKey {5, '\0'};

    auto table = readStickEntries();
    auto currentTable = table.end();

    client.broadcastHandler([&table, &currentTable](const std::string& , const std::string& msg) {
       //std::cout << "broadcast message receive from <"<<nick<<">\n";
        try {
            auto jsonMsg = nlohmann::json::parse(msg);
            auto broadcastMsg = jsonMsg.find("SongBroadcastMessage");
            if (broadcastMsg != jsonMsg.end()) {
                StickEntry tmpLastPosition;
                std::cout << "actual "
                          << broadcastMsg->at("songID")
                          << ":" << broadcastMsg->at("playlistID")
                          << " P: " << broadcastMsg->at("position") <<"\n";
                tmpLastPosition.m_titleID = broadcastMsg->at("songID");
                tmpLastPosition.m_albumID = broadcastMsg->at("playlistID");
                tmpLastPosition.m_position = broadcastMsg->at("position");
                bool isPlaying = broadcastMsg->at("playing");
                if (isPlaying && currentTable != table.end()) {
                    if (currentTable->m_albumID == tmpLastPosition.m_albumID) {
                        currentTable->m_titleID = tmpLastPosition.m_titleID;
                        if (tmpLastPosition.m_position > 0 && tmpLastPosition.m_position < 10000)
                            currentTable->m_position = tmpLastPosition.m_position;
                    }
                    else {
                        std::cout << "new album detected\n";
                        if (currentTable->m_stickType == StickFunctionality::freeStick) {
                            std::cout << "free stick inserted, bind song and album id\n";
                            currentTable->m_albumID = tmpLastPosition.m_albumID;
                            currentTable->m_titleID = tmpLastPosition.m_titleID;
                            if (tmpLastPosition.m_position > 0 && tmpLastPosition.m_position < 10000)
                                currentTable->m_position = tmpLastPosition.m_position;
                        }
                        else {
                            currentTable = std::end(table);
                        }
                    }
                }
            }
        } catch (std::exception& ex) {
            std::cerr << "exception <"<<ex.what() << ">\n"<<msg<<"\n";
            return;
        }
    });

    rfid.init();

    Debounce key;
    key.setNewCardHandler([&table, &client, &currentTable](std::vector<uint8_t> cardId) {
        std::cout << "New card detected with ID [";
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

            Action action { Action::release };

            // is this a release or a connect?
            if (id != 0x0) {
                action = Action::connect;
            } else {
                action = Action::release; // just to be clear here
                id = currentTable->m_id;
            }

            std::cout << "value 0x"<<std::hex<<id<<" try to find entry\n";
            currentTable = std::find_if(std::begin(table), std::end(table), [&id](const auto& elem) { return elem.m_id == id; });

            if (currentTable != std::end(table)) {

                // TODO when this is a release and the album does not match to the stick entry and this is not a free stick, do nothing

                nlohmann::json message;
                nlohmann::json data;
                if (action == Action::release) {
                    std::cout << "found! stop it\n";
                    message["CmdMsg"] = "stop";

                    data["album"] = currentTable->m_albumID;
                    data["title"] = currentTable->m_titleID;
                    data["position"] = currentTable->m_position;
                    message["data"] = data;

                    currentTable = std::end(table);
                    // TODO write table to file
                    std::cout << "msg: "<<message.dump(2)<<"\n";
                    client.send(snc::Client::SendType::cl_send, "audioserver", message.dump());
                }
                else {
                    std::cout << "found! start it\n";
                    currentTable = std::find_if(std::begin(table), std::end(table), [&id](const auto& elem) { return elem.m_id == id; });
                    if (currentTable != std::end(table)) {
                        message["CmdMsg"] = "start";

                        data["album"] = currentTable->m_albumID;
                        data["title"] = currentTable->m_titleID;
                        data["position"] = currentTable->m_position;
                        message["data"] = data;

                        std::cout << "msg: "<<message.dump(2)<<"\n";
                        client.send(snc::Client::SendType::cl_send, "audioserver", message.dump());
                    }
                }
            }
            else {
                std::cout << "unknown stick id\n";
            }
        }

    });

    auto rfidCaller =[&rfid, &key](){
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

    };

    RepeatTimer repeatTimer(ioc, 50ms);
    repeatTimer.setHandler(std::move(rfidCaller));
    repeatTimer.start();

    ioc.run();

    return 0;

}

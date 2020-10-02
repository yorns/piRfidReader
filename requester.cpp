#include <iostream>

#include <snc/client.h>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>

using namespace std::chrono_literals;

int main(int argc, char* argv[])
{

    if (argc != 3 && argc != 2) {

        std::cerr << "usage "<< argv[0] << " <Album Name> <Song Name>\n";
        std::cerr << "usage "<< argv[0] << " stop\n";
        return -1;

    }

    boost::asio::io_context ioc;
    snc::Client client("requester", ioc, "127.0.0.1", 12001);

    nlohmann::json msg;
    bool sendout { false };

    if (argc == 2) {
        std::cout << "test for command stop\n";
        std::string command { argv[1] };
        if (command == "stop") {
            msg["CmdMsg"] = "stop";
            sendout = true;
        }
    }
    else {
        std::cout << "test for command start\n";
        std::string albumInfo { argv[1] };
        std::string titleInfo { argv[2] };

        nlohmann::json request;

        msg["CmdMsg"] = "start";

        request["album"] = albumInfo;
        request["title"] = titleInfo;
        request["position"] = 0;

        msg["data"] = request;
        sendout = true;
    }

    if (sendout) {
        client.send(snc::Client::SendType::cl_send, "audioserver", msg.dump());
        client.recvHandler([](const std::string& nick, const std::string& msg) {

            std::cout << "received from <"<<nick<<">: " << msg << "\n";

        });

        boost::asio::steady_timer timeout(ioc, 5s);
        timeout.async_wait([&client](const boost::system::error_code& ) {
            client.stop();
        });

        ioc.run();

    }
}

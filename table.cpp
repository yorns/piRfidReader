#include "table.h"

std::string Table::defaultconfigFileName = std::string("/var/audioserver/stick/keyTable.json");

Table::Table(std::string configFileName) {
    if (configFileName.empty()) {
        m_configFileName = defaultconfigFileName;
    }
    else {
        m_configFileName = configFileName;
    }
    m_currentStickItem = m_table.end();
}

bool Table::hasCurrent() {
    return m_currentStickItem != m_table.end();
}

StickEntry &Table::getCurrent() {
    return *m_currentStickItem;
}

bool Table::setCurrent(uint64_t keyID) {
    auto oldCurrent = m_currentStickItem;
    m_currentStickItem =
            std::find_if(std::begin(m_table),
                         std::end(m_table),
                         [&keyID](const auto& elem) { return elem.getKeyID() == keyID; });

    return m_currentStickItem != oldCurrent;
}

void Table::unsetCurrent() {
    m_currentStickItem = std::end(m_table);
}


std::optional<StickEntry> Table::find(uint64_t keyID) {
    auto iter = std::find_if(std::begin(m_table), std::end(m_table), [&keyID](const auto& elem) { return elem.getKeyID() == keyID; });

    if (iter != std::end(m_table)) {
        std::cout << "found entry for key id <0x"<<std::hex<<keyID<<std::dec<<">\n";
        return *iter;
    }
    else {
        std::cout << "NO entry found for key id <0x"<<std::hex<<keyID<<std::dec<<">\n";
        return std::nullopt;
    }
}

void Table::readStickDatabase()
{
    std::cout << "read database information at: <"<<m_configFileName<<">\n";
    std::ifstream ifs(m_configFileName);

    if (ifs.good()) {
        nlohmann::json data = nlohmann::json::parse(ifs);

        for (const auto& entry : data) {
            StickEntry item;
            if (item.fromJson(entry))
                m_table.emplace_back(std::move(item));
        }
    }
    else {
        std::cerr << "config file not found\n";
    }

    m_currentStickItem = m_table.end();
}

void Table::writeStickEntries() {
    nlohmann::json jsonfile;

    for (const auto& elem : m_table) {
        jsonfile.push_back(elem.toJson());
    }

    std::ofstream output(m_configFileName);
    output << jsonfile.dump(2);
    output.close();
}

bool Table::updateFromJson(const nlohmann::json &broadcastMsg) {
    StickEntry currentSongItem;
    bool isPlaying {false};

    try {
        std::cout << "received current song/album ID info " << broadcastMsg.at("songID")
                  << ":" << broadcastMsg.at("playlistID")
                  << " Pos: " << broadcastMsg.at("position") <<"\n";

        currentSongItem.setStickNameType(StickNameType::isBoth);
        currentSongItem.setTitleID(broadcastMsg.at("songID"));
        currentSongItem.setTitle(broadcastMsg.at("title"));
        currentSongItem.setAlbumID(broadcastMsg.at("playlistID"));
        currentSongItem.setAlbum(broadcastMsg.at("album"));
        currentSongItem.setPosition(broadcastMsg.at("position"));

        isPlaying = broadcastMsg.at("playing");

    } catch (std::exception& ex) {
        std::cerr << "error: " << ex.what() << "\ncannot parse broadcast message: " << broadcastMsg.dump() <<"\n";
        return false;
    }

    // does the currently used stick has an entry within the table
    if (m_currentStickItem != m_table.end()) {
        // is the player playing, then only update the title and position
        if (isPlaying) {
            if (m_currentStickItem->isSameAlbum(currentSongItem)) {
                if ( m_currentStickItem->getStickNameType() == StickNameType::isID ||
                     m_currentStickItem->getStickNameType() == StickNameType::isBoth) {
                    m_currentStickItem->setTitleID(currentSongItem.getTitleID());
                }
                else if (m_currentStickItem->getStickNameType() == StickNameType::isName ||
                         m_currentStickItem->getStickNameType() == StickNameType::isBoth) {
                    m_currentStickItem->setTitle(currentSongItem.getTitle());
                }
                if (currentSongItem.getPosition() > 0 && currentSongItem.getPosition() < 10000)
                    m_currentStickItem->setPosition(currentSongItem.getPosition());
                return true;
            }
            else {
                std::cout << "new album detected\n";
                // if this is a free stick, store album, title and
                if (m_currentStickItem->getStickFunction() == StickFunctionality::freeStick) {
                    std::cout << "free stick inserted, bind song and album id\n";
                    if (m_currentStickItem->getStickNameType() == StickNameType::isID ||
                            m_currentStickItem->getStickNameType() == StickNameType::isBoth) {
                        m_currentStickItem->setAlbumID(currentSongItem.getAlbumID());
                        m_currentStickItem->setTitleID(currentSongItem.getTitleID());
                        if (currentSongItem.getPosition() > 0 && currentSongItem.getPosition() < 10000)
                            m_currentStickItem->setPosition(currentSongItem.getPosition());
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

#include "stickentry.h"


StickEntry::StickEntry(uint64_t id, std::string &&albumID, std::string &&titleID, uint32_t position, StickFunctionality stick, StickNameType stickNameType)
    : m_id(id),
      m_albumID(std::move(albumID)),
      m_titleID(std::move(titleID)),
      m_position(position),
      m_stickType(stick),
      m_stickNameType(stickNameType) {}

void StickEntry::setKeyID(const uint64_t keyID) {
    m_id = keyID;
}

void StickEntry::setAlbumID(const std::string &albumID) {
    m_albumID = albumID;
}

void StickEntry::setAlbum(const std::string &album) {
    m_album = album;
}

void StickEntry::setTitle(const std::string &title) {
    m_title = title;
}

void StickEntry::setTitleID(const std::string &titleID) {
    m_titleID = titleID;
}

void StickEntry::setPosition(uint32_t position) {
    m_position = position;
}

void StickEntry::setStickFunction(StickFunctionality stickFunctionality) {
    m_stickType = stickFunctionality;
}

void StickEntry::setStickNameType(StickNameType stickNameType) {
    m_stickNameType = stickNameType;
}

uint64_t StickEntry::getKeyID() const {
    return m_id;
}

std::string StickEntry::getAlbumID() const {
    return m_albumID;
}

std::string StickEntry::getAlbum() const {
    return m_album;
}

std::string StickEntry::getTitleID() const {
    return m_titleID;
}

std::string StickEntry::getTitle() const {
    return m_title;
}

StickFunctionality StickEntry::getStickFunction() const {
    return m_stickType;
}

StickNameType StickEntry::getStickNameType() const {
    return m_stickNameType;
}

uint32_t StickEntry::getPosition() const {
    return m_position;
}

bool StickEntry::isSameAlbum(const StickEntry &entry) {

    if ((entry.getStickNameType() == StickNameType::isBoth ||
         entry.getStickNameType() == StickNameType::isID ) &&
            (m_stickNameType == StickNameType::isID ||
             m_stickNameType == StickNameType::isBoth) &&
            entry.getAlbumID() == m_albumID) {
        return true;
    }

    if ((entry.getStickNameType() == StickNameType::isName ||
         entry.getStickNameType() == StickNameType::isBoth) &&
            (m_stickNameType == StickNameType::isName ||
             m_stickNameType == StickNameType::isBoth) &&
            entry.getAlbum() == m_album) {
        return true;
    }

    return false;
}

nlohmann::json StickEntry::toJson() const {
    nlohmann::json entry;
    entry["id"] = m_id;

    if (!m_album.empty() && m_stickNameType == StickNameType::isName)
        entry["album"] = m_album;
    else if (!m_albumID.empty() && m_stickNameType == StickNameType::isID)
        entry["albumID"] = m_albumID;
    else
        std::cerr << "no album information given\n";

    if (!m_title.empty() && m_stickNameType == StickNameType::isName)
        entry["title"] = m_title;
    else if (!m_titleID.empty() && m_stickNameType == StickNameType::isID)
        entry["titleID"] = m_titleID;
    else
        std::cerr << "no matching title information given\n";

    entry["position"] = m_position;

    entry["freeStick"] = m_stickType == StickFunctionality::freeStick;

    return entry;
}

bool StickEntry::fromJson(const nlohmann::json &entry) {
    try {
        m_id = entry.at("id");
        if (entry.find("album") != entry.end() && m_stickNameType != StickNameType::isID) {
            m_album = entry.at("album");
            m_stickNameType = StickNameType::isName;
        }
        else if (entry.find("albumID") != entry.end() && m_stickNameType != StickNameType::isName) {
            m_albumID = entry.at("albumID");
            m_stickNameType = StickNameType::isID;
        } else { std::cerr << "entry is missing album information\n"; }

        if (entry.find("title") != entry.end() && m_stickNameType != StickNameType::isID) {
            m_title = entry.at("title");
            m_stickNameType = StickNameType::isName;
        }
        else if (entry.find("titleID") != entry.end() && m_stickNameType != StickNameType::isName) {
            m_titleID = entry.at("titleID");
            m_stickNameType = StickNameType::isID;
        } else { std::cerr << "entry is missing title information\n"; }

        m_position = entry.at("postion");
        m_stickType = entry.at("freeStick")?StickFunctionality::freeStick:StickFunctionality::boundStick;

    } catch(std::exception& ex) {
        std::cerr << "Error ("<<ex.what()<<") decoding of \n"<<entry.dump(2)<<"\n";
        return false;
    }
    return true;
}

bool StickEntry::hasID() { return m_stickNameType == StickNameType::isID; }

bool StickEntry::hasName() { return m_stickNameType == StickNameType::isName; }

nlohmann::json StickEntry::generateCmdMsg(Action action) {

    std::string actionCmd;
    if (action == Action::connect) {
        actionCmd = "start";
    }
    else if (action == Action::release) {
        actionCmd = "stop";
    }


    nlohmann::json message;
    nlohmann::json data;

    message["CmdMsg"] = actionCmd;

    if ( m_stickNameType == StickNameType::isID ||
         m_stickNameType == StickNameType::isBoth) {
        data["albumID"] = m_albumID;
        data["titleID"] = m_titleID;
    }
    else {
        data["album"] = m_album;
        data["title"] = m_title;
    }
    \

    data["position"] = m_position;
    message["data"] = data;

    return message;
}

std::string StickEntry::dump() {
    auto msg = toJson();
    return msg.dump(2);
}

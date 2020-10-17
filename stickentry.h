#ifndef STICKENTRY_H
#define STICKENTRY_H

#include <inttypes.h>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <iostream>

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
    isName,
    isBoth,
    unset
};

class StickEntry {
    uint64_t m_id;
    std::string m_albumID;
    std::string m_album;
    std::string m_titleID;
    std::string m_title;
    uint32_t m_position {0};
    StickFunctionality m_stickType { StickFunctionality::boundStick };
    StickNameType m_stickNameType { StickNameType::unset };

public:
    StickEntry(uint64_t id, std::string&& albumID, std::string&& titleID,
               uint32_t position, StickFunctionality stick, StickNameType stickNameType);
    StickEntry() = default;

    bool hasID();
    bool hasName();

    void setKeyID(const uint64_t keyID);
    void setAlbumID(const std::string& albumID);
    void setAlbum(const std::string& album);
    void setTitle(const std::string& title);
    void setTitleID(const std::string& titleID);
    void setPosition( uint32_t position);
    void setStickFunction(StickFunctionality stickFunctionality);
    void setStickNameType(StickNameType stickNameType);

    uint64_t getKeyID() const;
    std::string getAlbumID() const;
    std::string getAlbum() const;
    std::string getTitleID() const;
    std::string getTitle() const;
    StickFunctionality getStickFunction() const;
    StickNameType getStickNameType() const;
    uint32_t getPosition() const;

    bool isSameAlbum(const StickEntry& entry);

    nlohmann::json toJson() const;
    bool fromJson(const nlohmann::json& entry);

    nlohmann::json generateCmdMsg(Action action);

    std::string dump();

};

#endif // STICKENTRY_H

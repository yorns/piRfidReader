#ifndef TABLE_H
#define TABLE_H

#include <vector>
#include <fstream>
#include <nlohmann/json.hpp>
#include "stickentry.h"

class Table {

    static std::string defaultconfigFileName;

    std::string m_configFileName;

    std::vector<StickEntry> m_table;
    std::vector<StickEntry>::iterator m_currentStickItem;

public:
    Table(std::string configFileName = defaultconfigFileName);

    bool hasCurrent();
    StickEntry& getCurrent();

    std::optional<StickEntry> find(uint64_t keyID);

    void readStickDatabase();
    void writeStickEntries();

    //! updateFromJson
    //! update a table entry if it is available or create a new entry if stick is free
    //! \brief updateFromJson
    //! \param broadcastMsg system message about current status
    //! \return
    //!
    bool updateFromJson(const nlohmann::json& broadcastMsg);


};

#endif // TABLE_H

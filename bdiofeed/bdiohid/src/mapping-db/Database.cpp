#include "database.h"
#include "reader.h"
#include "SQLiteCpp/SQLiteCpp.h"

void Database::Clear()
{
    // First, check for 'all'. If we're clearing everything, we don't have to clear anything else.
    for (const auto& clear : m_clears)
    {
        if (!strcasecmp(clear.c_str(), "all"))
        {
            // clearing everything is the same as deleting the file and recreating it.
            unlink(m_filename.c_str());
            EnsureTablesExist();
            return;
        }
    }

    // It's weird, but it's possible that we're trying to clear specific stuff from a nonexistent database.
    // Make sure the database exists.
    EnsureTablesExist();

    // If we got here, we're not clearing everything. Build a list of the devices to be erased.
    bool seenAuto = false;
    std::vector<std::pair<std::string, std::string>> devicesToClear;
    for (const auto& clear : m_clears)
    {
        if (!strcasecmp(clear.c_str(), "settings"))
        {
            SQLite::Database database(m_filename);
            SQLite::Statement drop(database, "DROP TABLE Settings;");
            drop.exec();
        }
        else if (!strcasecmp(clear.c_str(), "auto"))
        {
            if (!seenAuto)
            {
                seenAuto = true;
                for (const auto& device : Reader::s_displays)
                {
                    devicesToClear.push_back(std::make_pair(device.ttyDriver, device.ttyModel));
                }
            }
        }
        else
        {
            auto slash = clear.find('/');
            if (slash != std::string::npos)
            {
                devicesToClear.push_back(std::make_pair(clear.substr(0, slash), clear.substr(slash+1)));
            }
            else 
            {
                devicesToClear.push_back(std::make_pair(clear, ""));
            }
        }       
    }

    // Now clear the devices.
    {
        SQLite::Database database(m_filename);
        std::vector<int> modelIds;
        for (const auto &device : devicesToClear)
        {
            std::string queryText = "SELECT modelid FROM displays WHERE mfgr=?;";
            if (!device.second.empty())
            {
                queryText = "SELECT modelid FROM displays WHERE mfgr=? AND model=?;";
            }

            SQLite::Statement query(database, queryText.c_str());
            query.bind(1, device.first.c_str());
            if (!device.second.empty())
            {
                query.bind(2, device.second.c_str());
            }
            while (query.executeStep())
            {
                modelIds.push_back(query.getColumn(0).getInt());
            }
        }

        for (const auto modelId : modelIds)
        {
            SQLite::Statement displayClear(database, "DELETE FROM displays WHERE modelid=?;");
            displayClear.bind(1, modelId);
            displayClear.exec();

            SQLite::Statement collectionsClear(database, "DELETE FROM collections WHERE modelid=?;");
            collectionsClear.bind(1, modelId);
            collectionsClear.exec();

            SQLite::Statement mappingsClear(database, "DELETE FROM mappings WHERE modelid=?;");
            mappingsClear.bind(1, modelId);
            mappingsClear.exec();
        }
    }
}
void Database::Write()
{

}

void Database::EnsureTablesExist()
{
    SQLite::Database database(m_filename);
    if (!database.tableExists("settings"))
    {
        SQLite::Statement query(database, "CREATE TABLE settings(setting_name, setting_value);");
        query.exec();
    }

    if (!database.tableExists("displays"))
    {
        SQLite::Statement query(database, "CREATE TABLE displays(modelid INTEGER PRIMARY KEY, mfgr, model, mfgr_name, model_name);");
        query.exec();
    }

    if (!database.tableExists("collections"))
    {
        SQLite::Statement query(database, "CREATE TABLE collections(modelid, collection_id, parent_collection_id, usage_page, usage);");
        query.exec();
    }

    if (!database.tableExists("mappings"))
    {
        SQLite::Statement query(database, "CREATE TABLE mappings(modelid, key_group, key_index, collection_id, usage_page, usage, name);");
        query.exec();
    }
}
#if 0
    std::string m_filename;
    std::vector<std::string> m_clears;
#endif
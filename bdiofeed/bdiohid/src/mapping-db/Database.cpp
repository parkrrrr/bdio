#include "Database.h"
#include "Reader.h"

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
            SQLite::Database database(m_filename, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
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
        SQLite::Database database(m_filename, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
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
    EnsureTablesExist();

    SQLite::Database database(m_filename, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    
    // write the settings 
    for (const auto& setting : Reader::s_settings)
    {
	SQLite::Statement writeSetting(database, "INSERT INTO settings VALUES (?,?);");
	writeSetting.bind(1, setting.name);
	writeSetting.bind(2, setting.value);
	writeSetting.exec();
    }

    // write the displays

    int displayId = 1;
    for (auto& display : Reader::s_displays)
    {
	std::cout << "Writing display " << display.manufacturer << " " << display.model << std::endl;
	SQLite::Statement writeDisplay(database, "INSERT INTO displays VALUES (?, ?, ?, ?, ?);");
        writeDisplay.bind(1, displayId);
        writeDisplay.bind(2, display.ttyDriver);
	writeDisplay.bind(3, display.ttyModel);
	writeDisplay.bind(4, display.manufacturer);
	writeDisplay.bind(5, display.model);
	writeDisplay.exec();

	int collectionId = 0;
	int mappingId = 0;
        WriteCollection(database, displayId, display.baseCollection, collectionId, -1, mappingId);        

	++displayId;	
    } 
}

void Database::WriteCollection( SQLite::Database& database, int displayId, Collection& collection, 
		int &collectionId, int parentCollectionId, int& mappingId)
{
    SQLite::Statement writeCollection(database, "INSERT INTO collections VALUES (?, ?, ?, ?, ?);");
    writeCollection.bind(1, displayId);
    writeCollection.bind(2, collectionId);
    writeCollection.bind(3, parentCollectionId);
    writeCollection.bind(4, collection.usage.usagePage);
    writeCollection.bind(5, collection.usage.usageId);
    writeCollection.exec();

    if (collection.cells)
    {
	Mapping cellMapping;
	cellMapping.usage.usagePage = 0x41;
	cellMapping.usage.usageId = (collection.cells == 6 ? 4 : 3);
        cellMapping.name = "cells";
        WriteMapping(database, displayId, collectionId, cellMapping, mappingId);
    }	

    for (auto& mapping : collection.mappings)
    {
        WriteMapping(database, displayId, collectionId, mapping, mappingId);
    }

    int myId = collectionId;
    ++collectionId;

    for (auto& subCollection : collection.subCollections)
    {
        WriteCollection(database, displayId, subCollection, collectionId, myId, mappingId);
    }
}

void Database::WriteMapping( SQLite::Database& database, int displayId, int collectionId, Mapping& mapping, 
		int& mappingId)
{
    SQLite::Statement writeMapping(database, "INSERT INTO mappings VALUES (?, ?, ?, ?, ?, ?, ?);");
    writeMapping.bind(1, displayId);
    writeMapping.bind(2, mapping.ttyGroup);
    writeMapping.bind(3, mapping.ttyKey);
    writeMapping.bind(4, collectionId);
    writeMapping.bind(5, mapping.usage.usagePage);
    writeMapping.bind(6, mapping.usage.usageId);
    writeMapping.bind(7, mapping.name);
    writeMapping.exec();
    mappingId++;
}

void Database::EnsureTablesExist()
{
    SQLite::Database database(m_filename, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
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

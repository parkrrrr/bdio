#pragma once
#include <vector>
#include <string>
#include "SQLiteCpp/SQLiteCpp.h"
#include "maptodb.h" 

class Database
{
public:
    Database(std::string filename, const std::vector<std::string>& clears) :
        m_filename(filename), 
        m_clears(clears)
        {}

    void Clear();
    void Write();

private:
    void EnsureTablesExist();

    void WriteCollection( SQLite::Database& database, int displayId, Collection& collection,
		                int &collectionId, int parentCollectionid, int& mappingId);
    void WriteMapping( SQLite::Database& database, int displayId, int collectionId, Mapping& mapping,
		                int& mappingId);

    std::string m_filename;
    std::vector<std::string> m_clears;
};

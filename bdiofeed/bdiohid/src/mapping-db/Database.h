#pragma once
#include <vector>
#include <string>

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

    std::string m_filename;
    std::vector<std::string> m_clears;
};

#include "maptodb.h"
#include "reader.h"
#include "database.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

void PrintCollection(Collection& collection, std::string istr = "")
{
    std::cout << istr << "Collection " << collection.usage.usagePage << " " << collection.usage.usageId << std::endl;
    istr += "  ";
    for (auto& c : collection.subCollections)
    {
        PrintCollection(c, istr);
    }
    for (auto& m : collection.mappings)
    {
        std::cout << istr << "Mapping " << m.usage.usagePage << " " << m.usage.usageId << " " << m.ttyGroup << " " << m.ttyKey << " " << m.name << std::endl;
    }
}

int main(int argc, char** argv)
{
    std::string databaseName;
    std::vector<std::string> clears;
    std::vector<std::string> filenames;

    bool gettingOpts = true;
    while (gettingOpts)
    {
        int opt = getopt(argc, argv, "c:o:");
        switch (opt)
        {
            case -1: 
                gettingOpts = false;
                break;

            case 'c':
                clears.push_back(optarg);
                break;

            case 'o':
                if (databaseName.empty())
                {
                    databaseName = optarg;
                }
                else
                {
                    std::cerr << "Only one database name may be specified." << std::endl;
                    return 1;
                }
                
                break;

            case '?':
                std::cerr << 
R"**(
Usage: maptodb <flags> <filename>...

Flags:
-c all | settings | smart | <driver> | <driver>/<model>
-o <output-database>
)**";
                return 1;
        }
    }

    for (int i = optind; i < argc; ++i)
    {
        filenames.push_back(argv[i]);
    }

    if (databaseName.empty())
    {
        std::cerr << "No database name specified (-o)" << std::endl;
        return 1;
    }

    Reader reader;
    while (!filenames.empty())
    {
        std::vector<std::string> newfilenames;

        for (const auto& name : filenames)
        {
            struct stat statResult;
            if (!stat(name.c_str(), &statResult))
            {
                if (S_ISDIR(statResult.st_mode))
                {
                    auto dir = opendir(name.c_str());
                    if (dir)
                    {
                        while (const auto entry = readdir(dir))
                        {   
                            // skip current/parent links, as well as any dotfiles.
                            if (!entry->d_name || !entry->d_name[0] || entry->d_name[0] == '.')
                            {
                                continue;
                            }
                            std::cout << "extra file " << name << " " << entry->d_name << std::endl;
                            newfilenames.push_back((name + "/") + entry->d_name);
                        }
                        closedir(dir);
                    }
                    std::cerr << "errno " << errno << std::endl;
                }
                else if (S_ISREG(statResult.st_mode))
                {
                    if (name.find(".settings") != name.npos)
                    {
                        std::cout << "Reading settings file \"" << name << "\"" << std::endl;
                        if (!reader.ReadSettingsFile(name))
                        {
                            return 1;
                        }
                    }
                    else
                    {
                        std::cout << "Reading device file \"" << name << "\"" << std::endl;
                        if (!reader.ReadDeviceFile(name))
                        {
                            return 1;
                        }
                    }                
                }
            }
        }
        filenames.clear();
        filenames = std::move(newfilenames);
    }
    
    return 0;
}


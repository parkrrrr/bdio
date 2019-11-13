#include "reports.h"
#include <cstdlib>

void Reports::SetButton(bool press, uint8_t group, uint8_t key)
{
    // find button in m_buttons, if present, using group and key
    bool keyIsIndex = false;

    auto button = m_buttons.find(std::make_pair(group, key));
    if (button == m_buttons.end())
    {
        keyIsIndex = true;
        button = m_buttons.find(std::make_pair(group, -1));
        if (button == m_buttons.end())
        {
            return;
        }
    }

    // get reference to appropriate byte in input report
    auto byteOffset = button->second.byteOffsetInInputReport;
    auto bit = button->second.bitInInputByte;

    if (keyIsIndex)
    {
        unsigned int bitOffset = (byteOffset << 3 ) + bit + key;
        byteOffset = static_cast<uint16_t>(bitOffset >> 3);
        bit = static_cast<uint8_t>(bitOffset & 7);
    }

    // set or unset bit in byte based on value of 'press'
    auto bitmask = 1 << bit;
    if (press)
    {
        m_inputReport[byteOffset] |= bitmask;
    }
    else
    {
        m_inputReport[byteOffset] &= ~bitmask;
    }
}

void Reports::SetOutputReport(const ByteString& outputReport)
{
    // extract output data from output report and send to brlapi
    // Since currently the only output we have is cells, there's not much 
    //"extracting" to do.
    m_brailleApi.WriteCells(outputReport);
}

void Reports::CreateReportModel()
{
    // open database
    SQLite::Database database(c_databaseName);

    // read display info from 'displays' table
    const auto& driverName = m_brailleApi.GetDriverName();
    const auto& modelName = m_brailleApi.GetModelName();

    m_cellCount = m_brailleApi.GetCellCount();

    SQLite::Statement displayQuery(database, "SELECT modelid, mfgr_name, model_name FROM displays WHERE mfgr=? and model=?;");
    displayQuery.bind(1, driverName.c_str());
    displayQuery.bind(2, modelName.c_str());
    
    if (displayQuery.executeStep())
    {
        m_modelId = displayQuery.getColumn(0).getInt();
        m_manufacturerName = displayQuery.getColumn(1).getString();
        m_modelName = displayQuery.getColumn(2).getString();
    }

    // read collection info from 'collections' and 'mappings' tables and build data structure
    Collection baseCollection;
    ReadCollection(database, baseCollection);

    // create report descriptor
    CreateReportDescriptorForCollection(baseCollection);

    // initialize input report
    m_inputReport = ByteString(m_maxReportSize, '\0');
    if (m_cellCountOffsetInInputReport >= 0)
    {
        m_inputReport[m_cellCountOffsetInInputReport] = m_cellCount;
    }
}

void Reports::ReadCollection(SQLite::Database& database, Collection& collection)
{
    // read information about base collection
    SQLite::Statement collectionQuery(database, "SELECT usage_page, usage FROM collections WHERE modelid = ? AND collection_id = ?;");
    collectionQuery.bind(1, m_modelId);
    collectionQuery.bind(2, collection.id);

    if (collectionQuery.executeStep())
    {
        collection.usage_page = MapUsagePage(collectionQuery.getColumn(0).getString());
        collection.usage = MapUsage(collection.usage_page, collectionQuery.getColumn(1).getString());
    }

    // read button mappings for this collection
    ReadButtonMappings(database, collection);

    // recursively read subcollections of this collection
    SQLite::Statement childQuery(database, "SELECT collection_id FROM collections WHERE parent_collection_id=?;");
    childQuery.bind(1, collection.id);

    while (childQuery.executeStep())
    {
        int childId = childQuery.getColumn(0).getInt();
        collection.subCollections.emplace_back(Collection{.id=childId});        
    }

    for (auto& subCollection : collection.subCollections)
    {
        ReadCollection(database, subCollection);
    }
}

void Reports::ReadButtonMappings(SQLite::Database& database, Collection& collection)
{
    SQLite::Statement buttonQuery(database, "SELECT key_group, key_index, usage_page, usage, name FROM mappings WHERE collection_id=?;");
    buttonQuery.bind(1, collection.id);

    while (buttonQuery.executeStep())
    {
        ButtonMapping newButton;
        newButton.key_group = buttonQuery.getColumn(0).getInt();
        newButton.key_index = buttonQuery.getColumn(1).getInt();
        newButton.usage_page = MapUsagePage(buttonQuery.getColumn(2).getString());
        newButton.usage = MapUsage(newButton.usage_page, buttonQuery.getColumn(3).getString());
        newButton.name = buttonQuery.getColumn(4).getString();

        collection.buttonMappings.emplace_back(std::move(newButton));
    }
}

void Reports::CreateReportDescriptorForCollection(Collection& baseCollection)
{
    // all fields have the same logical minimum, 0, so emit it once and never again.
    EmitByte(0x15);
    EmitByte(0x00);

    EmitCollection(baseCollection);
    EmitPaddingIfNeeded(0x81, 8);

    m_inputReportSize = m_inputReportBitSize / 8;
    m_maxReportSize = m_outputReportSize > m_inputReportSize ? m_outputReportSize : m_inputReportSize;
}

void Reports::EmitByte(uint8_t byte)
{
    m_reportDescriptor += byte;
}

void Reports::EmitWord(uint16_t word)
{
    EmitByte(word & 0xff);
    EmitByte(word >> 8);
};

void Reports::EmitUsagePage(uint16_t usagePage)
{
    if (usagePage != m_currentUsagePage)
    {
        if (usagePage > 0xff)
        {
            EmitByte(0x06);
            EmitWord(usagePage);
        }
        else 
        {
            EmitByte(0x05);
            EmitByte(usagePage);
        }
        m_currentUsagePage = usagePage;
    }
};

void Reports::EmitLogicalMaximum(uint8_t logicalMaximum)
{
    if (logicalMaximum != m_currentLogicalMaximum)
    {
        EmitByte(0x25);
        EmitByte(logicalMaximum);
        m_currentLogicalMaximum = logicalMaximum;
    }
};

void Reports::EmitReportSize(uint8_t reportSize)
{
    if (reportSize != m_currentReportSize)
    {
        EmitByte(0x75);
        EmitByte(reportSize);
        m_currentReportSize = reportSize;
    }
};

void Reports::EmitReportCount(uint8_t reportCount)
{
    if (reportCount != m_currentReportCount)
    {
        EmitByte(0x95);
        EmitByte(reportCount);
        m_currentReportCount = reportCount;
    }
};

void Reports::EmitUsage(uint16_t usagePage, uint8_t usage)
{
    EmitUsagePage(usagePage);
    if (usage > 0xff)
    {
        EmitByte(0x0a);
        EmitWord(usage);
    }
    else
    {
        EmitByte(0x09);
        EmitByte(usage);
    }        
};

void Reports::EmitString(std::string name)
{
    if (!name.empty())
    {
        m_strings.insert({m_stringId, name});
        if (m_stringId > 0xff)
        {
            EmitByte(0x7a);
            EmitWord(m_stringId);
        }
        else
        {
            EmitByte(0x79);
            EmitByte(m_stringId);
        }
        ++m_stringId;            
    }
};

void Reports::EmitPaddingIfNeeded(uint8_t tag, uint8_t reportSize)
{        
    if (tag == 0x81 && reportSize != 1 && (m_inputReportBitSize & 7))
    {
        unsigned int bits = 8-(m_inputReportBitSize & 7);
        EmitReportSize(bits);
        EmitReportCount(1);
        EmitByte(tag);
        EmitByte(0x03); // constant, variable, absolute, no wrap, linear, preferred state, no null, bitfield
        m_inputReportBitSize += bits;
    }        
};

void Reports::EmitDataItem(const ButtonMapping& mapping)
{
    uint8_t tag = 0;
    uint8_t logicalMaximum = 0;
    uint8_t reportSize = 0;
    uint8_t reportCount = 0;
    bool isButton = false;

    if (mapping.usage_page == 0x41) // Braille usage page
    {
        if (mapping.usage == 0x03 || mapping.usage == 0x04) // cell8, cell6
        {
            if (mapping.usage == 0x03)
            {
                logicalMaximum = 0xff;
            }
            else
            {
                logicalMaximum =0x3f;
            }
            reportSize = 8;
            reportCount = m_cellCount;
            tag = 0x91; // output
        }
        else if (mapping.usage == 0x05) // cell count
        {
            logicalMaximum = m_cellCount;
            reportSize = 8;
            reportCount = 1;
            tag = 0x81; // input
        }
        else if (mapping.usage == 0x100) // router
        {
            logicalMaximum = 1;
            reportSize = 1;
            reportCount = m_cellCount;
            tag = 0x81; // input
            isButton = true;
        }
    }

    // all other usages are buttons
    if (!tag)
    {
        logicalMaximum = 1;
        reportSize = 1;
        reportCount = 1;
        tag = 0x81;
        isButton = true;
    }

    // if we need to emit a non-bit value to the input record, pad to a byte boundary
    EmitPaddingIfNeeded(tag, reportSize);

    // add button mappings, including routers, to ButtonInformation collection
    if (isButton)
    {
        m_buttons.insert
        (
            std::make_pair(
                std::make_pair<uint8_t, int16_t>(mapping.key_group, mapping.key_index), 
                ButtonInformation{mapping.key_group, mapping.key_index, static_cast<uint16_t>(m_inputReportBitSize>>3), static_cast<uint8_t>(m_inputReportBitSize & 7)}
            )
        );
    }

    if (mapping.usage == 0x05) 
    {
        m_cellCountOffsetInInputReport = m_inputReportBitSize / 8;
    }

    EmitLogicalMaximum(logicalMaximum);
    EmitReportSize(reportSize);
    EmitReportCount(reportCount);
    EmitString(mapping.name);
    EmitUsage(mapping.usage_page, mapping.usage);
    EmitByte(tag);
    EmitByte(0x02); // data, variable, absolute, no wrap, linear, preferred state, no null, (nonvolatile if output), bitfield
    
    // update position counters
    if (tag == 0x81)
    {
        m_inputReportBitSize += (reportSize * reportCount);
    }
    else if (tag == 0x91)
    {
        // the only output data we currently support is cells, so they're always the entire output report
        m_cellsOffsetInOutputReport = 0;
        m_outputReportSize = reportCount;
    }

    // if we emitted a non-bit value to the input record, pad to a byte boundary again if needed (should be only for routers)
    EmitPaddingIfNeeded(tag, reportSize);
}

void Reports::EmitCollection(const Collection& collection)
{
    // collection start
    EmitByte(0xa1);
    EmitByte(collection.id == 0 ? 0x01 : 0x02);  // application if collection 0, else logical

    // collection contents
    for (const auto& mapping : collection.buttonMappings)
    {
        EmitDataItem(mapping);
    }

    // sub-collections
    for (const auto& subCollection : collection.subCollections)
    {
        EmitCollection(subCollection);
    }

    // collection end
    EmitByte(0xc0);
};

uint16_t Reports::MapUsagePage(std::string pageName)
{
    static std::map<std::string, uint16_t> pages
    {
        {"braille", 0x41},
        {"button", 0x09},
    };

    auto page = pages.find(pageName);
    if (page != pages.end())
    {
        return page->second;
    }

    // we don't know this usage page.
    throw std::exception();
}

uint16_t Reports::MapUsage(uint16_t usagePage, std::string usageName)
{
    uint16_t usage = 0;

    switch (usagePage)
    {
        case 0x09: // buttons
        {
            usage = atoi(usageName.c_str());
            break;
        }

        case 0x41: // braille
        {
            static std::map<std::string, uint16_t> usages
            {
                // comments like the one below refer to sections in HUTRR-78 
                // [https://www.usb.org/sites/default/files/hutrr78_-_creation_of_a_braille_display_usage_page_0.pdf]
                // 20.1 Braille Display Device
                {"display", 0x1},    // display application collection

                // 20.2 Braille Cells
                {"row", 0x2},        // row collection (contains cells, cell count, router collections, router buttons)
                {"cell8", 0x3},      // 8-dot Braille cell (8 bits)
                {"cell6", 0x4},      // 6-dot Braille cell (8 bits)
                {"cellcount", 0x5},  // cell count (8 bits)

                // 20.3 Routers
                {"router1", 0xfa},   // router set 1
                {"router2", 0xfb},   // router set 2
                {"router3", 0xfc},   // router set 3
                {"router", 0x100},   // router key (contained in router set, array)

                // 20.4 Braille Buttons
                {"buttons", 0x200},  // generic Braille button collection
                
                {"dot1", 0x201} ,    // keyboard dot 1
                {"dot2", 0x202} ,    // keyboard dot 2
                {"dot3", 0x203} ,    // keyboard dot 3
                {"dot4", 0x204} ,    // keyboard dot 4
                {"dot5", 0x205} ,    // keyboard dot 5
                {"dot6", 0x206} ,    // keyboard dot 6
                {"dot7", 0x207} ,    // keyboard dot 7
                {"dot8", 0x208} ,    // keyboard dot 8
                
                {"space", 0x209} ,   // keyboard space
                {"lspace", 0x20a},   // keyboard left space
                {"rspace", 0x20b},   // keyboard right space
                
                {"face", 0x20c},     // face controls collection
                {"left", 0x20d},     // left controls collection
                {"right", 0x20e},    // right controls collection
                {"top", 0x20f},      // top controls collection

                {"jcenter", 0x210},  // joystick center
                {"jup", 0x211},      // joystick up
                {"jdown", 0x212},    // joystick down
                {"jleft", 0x213},    // joystick left
                {"jright", 0x214},   // joystick right

                {"dcenter", 0x215},  // d-pad center
                {"dup", 0x216},      // d-pad up
                {"ddown", 0x217},    // d-pad down
                {"dleft", 0x218},    // d-pad left
                {"dright" ,0x219},   // d-pad right

                {"pleft", 0x21a},    // pan left
                {"pright", 0x21b},   // pan right

                {"rup", 0x21c},      // rocker up
                {"rdown", 0x21d},    // rocker down
                {"rpress", 0x21e},   // rocker press
            };
 
            auto entry = usages.find(usageName);
            if (entry != usages.end())
            {
                usage = entry->second;
            }
            break;
        }
    }

    if (usage)
    {
        return usage;
    }

    // we don't know this usage
    throw std::exception();
}

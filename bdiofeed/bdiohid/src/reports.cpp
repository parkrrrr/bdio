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
    m_inputReport = ByteString(m_inputReportSize, '\0');
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
        collection.usage_page = collectionQuery.getColumn(0).getInt();
        collection.usage = collectionQuery.getColumn(1).getInt();
    }

    // read button mappings for this collection
    ReadButtonMappings(database, collection);

    // recursively read subcollections of this collection
    SQLite::Statement childQuery(database, "SELECT collection_id FROM collections WHERE modelid=? AND parent_collection_id=?;");
    childQuery.bind(1, m_modelId);
    childQuery.bind(2, collection.id);

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
    SQLite::Statement buttonQuery(database, "SELECT key_group, key_index, usage_page, usage, name FROM mappings WHERE modelid=? AND collection_id=?;");
    buttonQuery.bind(1, m_modelId);
    buttonQuery.bind(2, collection.id);

    while (buttonQuery.executeStep())
    {
        ButtonMapping newButton;
        newButton.key_group = buttonQuery.getColumn(0).getInt();
        newButton.key_index = buttonQuery.getColumn(1).getInt();
        newButton.usage_page = buttonQuery.getColumn(2).getInt();
        newButton.usage = buttonQuery.getColumn(3).getInt();
        newButton.name = buttonQuery.getColumn(4).getString();

        collection.buttonMappings.emplace_back(std::move(newButton));
    }
}

void Reports::CreateReportDescriptorForCollection(Collection& baseCollection)
{
    // all fields have the same logical minimum, 0, so emit it once and never again.
    EmitByte(0x15);
    EmitByte(0x00);

    EmitCollection(baseCollection, 8);

    m_inputReportSize = m_inputReportBitSize / 8;
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
        if (usagePage > 0U)
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

void Reports::EmitUsage(uint16_t usagePage, uint16_t usage)
{
    EmitUsagePage(usagePage);
    if (usage > 0xffU)
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
    // Linux gadget drivers don't support strings yet
    return;
    /*
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
    */
};

void Reports::EmitPaddingIfNeeded(uint8_t tag, uint8_t reportSize)
{        
    if (tag == 0x81 && reportSize > 1 && (m_inputReportBitSize & 7))
    {
        unsigned int bits = 8-(m_inputReportBitSize & 7);
        EmitUsage(0x9, 0xffff);
        EmitLogicalMaximum(1);
        EmitReportSize(1);
        EmitReportCount(bits);
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

void Reports::EmitCollection(const Collection& collection, uint8_t pad)
{
    // collection start
    EmitUsage(collection.usage_page, collection.usage);
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
        EmitCollection(subCollection, 0);
    }

    if (pad)
    {
        EmitPaddingIfNeeded(0x81, pad);
    }

    // collection end
    EmitByte(0xc0);
};


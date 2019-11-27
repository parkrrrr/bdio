#pragma once

#include "SQLiteCpp/SQLiteCpp.h"
#include "brlapi_interface.h"
#include <vector>
#include <map>

class Reports
{
public:
    Reports(BrlApiInterface& brailleApi) :
        m_brailleApi(brailleApi)
    {
        CreateReportModel();
    }

    ByteString GetReportDescriptor() const {return m_reportDescriptor;}
    std::string GetManufacturerName() const {return m_manufacturerName;}
    std::string GetModelname() const {return m_modelName;}

    const std::map<uint16_t, std::string>& GetStrings() const {return m_strings;}

    uint16_t GetInputReportSize() const {return m_inputReportSize;}
    uint16_t GetOutputReportSize() const {return m_outputReportSize;}

    ByteString GetInputReport() const {return m_inputReport;}
    void SetButton(bool press, uint8_t group, uint8_t key);

    void SetOutputReport(const ByteString& outputReport);

private:

    struct ButtonMapping
    {
        // db: modelid, key_group, key_index, collection_id, usage_page, usage, name
        // key_group == -1  ==>  cells, not keys
        // key_index == -1  ==>  array (cells, routing keys)
        int key_group = -2; 
        int key_index = -2;
        uint16_t usage_page = 0;
        uint16_t usage = 0;
        std::string name;
    };

    struct Collection
    {
        // db: modelid, collection_id, parent_collection_id, usage_page, usage
        int id = 0;
        uint16_t usage_page = 0;
        uint16_t usage = 0;
        std::vector<Collection> subCollections;
        std::vector<ButtonMapping> buttonMappings;            
    };

    void CreateReportModel();
    void ReadCollection(SQLite::Database& database, Collection& collection);
    void ReadButtonMappings(SQLite::Database& database, Collection& collection);
    void CreateReportDescriptorForCollection(Collection& collection);

    // function definitions for basic integer sizes
    void EmitByte(uint8_t byte);
    void EmitWord(uint16_t word);

    // *** begin global items (HID DCD 1.11 section 6.2.2.7)

    // usage page
    int m_currentUsagePage = -1;
    void EmitUsagePage(uint16_t usagePage);

    // cells and cell counts have different logical maxima from buttons, so track that.
    int m_currentLogicalMaximum = -1; 
    void EmitLogicalMaximum(uint8_t logicalMaximum);

    // physical minima and maxima and units are meaningless in Braille

    // report size
    int m_currentReportSize = -1;
    void EmitReportSize(uint8_t reportSize);

    // For now, everything is in the same report, so we don't use report IDs.

    // report count
    int m_currentReportCount = -1;
    void EmitReportCount(uint8_t reportCount);

    // we don't currently use push or pop

    // *** begin local items (HID DCD 1.11 section 6.2.2.8)

    // usage (we just go ahead and put usage page in this method, too)
    void EmitUsage(uint16_t usagePage, uint16_t usage);

    // We don't currently block adjacent usage IDs together, so min/max usage aren't used.
    // Designators are irrelevant to Braille.
    
    // string index
    uint16_t m_stringId = 4;  // 1, 2, and 3 are the mfgr, model, and serial number strings
    void EmitString(std::string name);

    // No min/max string index; see above
    // no aliases

    // *** begin function definitions for emitting collections and data items (main items)

    // Emit padding bits into the report specified by tag (0x81=input, 0x91=output, 0x93=feature)
    void EmitPaddingIfNeeded(uint8_t tag, uint8_t reportSize);

    // Emit the data item. This determines the appropriate report based on the usage.
    void EmitDataItem(const ButtonMapping& mapping);

    void EmitCollection(const Collection& collection, uint8_t pad);

    uint16_t MapUsagePage(std::string pageName);
    uint16_t MapUsage(uint16_t usagePage, std::string usageName);

    ByteString m_reportDescriptor;
    ByteString m_inputReport;

    int m_modelId = 0;
    unsigned int m_cellCount = 0;
    std::string m_manufacturerName;
    std::string m_modelName;

    std::map<uint16_t, std::string> m_strings;
    uint16_t m_outputReportSize = 0;
    unsigned int m_inputReportBitSize = 0;
    uint16_t m_inputReportSize = 0;

    BrlApiInterface& m_brailleApi;

    uint16_t m_cellsOffsetInOutputReport = 0;
    int m_cellCountOffsetInInputReport = -1;

    struct ButtonInformation
    {
        int brlApiGroup = 0;
        int brlApiId = 0;
        uint16_t byteOffsetInInputReport = 0;
        uint8_t bitInInputByte = 0;
    };
    

    // mapping from brlApiGroup/brlApiId to button information
    std::map<std::pair<uint8_t,int16_t>, ButtonInformation> m_buttons;
};
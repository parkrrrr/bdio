#pragma once

#include "SQLiteCpp/SQLiteCpp.h"
#include "brlapi_interface.h"

class Reports
{
    Reports(BrlApiInterface& brailleApi) :
        m_brailleApi(brailleApi)
    {
        CreateReportModel();
    }

    ByteString GetReportDescriptor() const {return m_reportDescriptor;}
    std::string GetManufacturerName() const {return m_manufacturerName;}
    std::string GetModelname() const {return m_modelName;}

    const std::map<uint16_t, std::string>& GetStrings() const {return m_strings;}

    uint16_t GetMaxReportSize() const {return m_maxReportSize;}

    ByteString GetInputReport() const {return m_inputReport;}
    void SetButton(bool press, uint8_t group, uint8_t key);

    void SetOutputReport(const ByteString& outputReport);

private:
    void CreateReportModel();
    void ReadCollection(SQLite::Database& database, Collection& collection);
    void ReadButtonMappings(SQLite::Database& database, Collection& collection);
    void CreateReportDescriptorForCollection(Collection& collection);

    uint16_t MapUsagePage(std::string& pageName);
    uint16_t MapUsage(uint16_t usagePage, std::string& usageName);

    ByteString m_reportDescriptor;
    ByteString m_inputReport;

    int m_modelId = 0;
    unsigned int m_cellCount = 0;
    std::string m_manufacturerName;
    std::string m_modelName;

    std::map<uint16_t, std::string> m_strings;
    uint16_t m_maxReportSize = 0;
    uint16_t m_outputReportSize = 0;
    unsigned int m_inputReportBitSize = 0;
    uint16_t m_inputReportSize = 0;

    BrlApiInterface& m_brailleApi;

    uint16_t m_cellsOffsetInOutputReport = 0;
    int m_cellCountOffsetInInputReport = -1;

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
    }

    struct Collection
    {
        // db: modelid, collection_id, parent_collection_id, usage_page, usage
        int id = 0;
        uint16_t usage_page = 0;
        uint16_t usage = 0;
        std::vector<Collection> subCollections;
        std::vector<ButtonMapping> buttonMappings;            
    };

    struct ButtonInformation
    {
        int brlApiGroup = 0;
        int brlApiId = 0;
        uint16_t byteOffsetInInputReport = 0;
        uint8_t bitMaskInInputByte = 0;
    };
    

    // mapping from brlApiGroup/brlApiId to button information
    std::map<std::pair<uint8_t,uint8_t>, ButtonInformation> m_buttons;
};
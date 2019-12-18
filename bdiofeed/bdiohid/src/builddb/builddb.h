#pragma once
#include <variant>
#include <string>
#include <vector>

class Application 
{
public:
    Application(std::string directory, std::string database) :
        m_directory(directory),
        m_database(database)
        {}

    void Run();

private:
    void WriteDatabase();
    void TraverseDirectory(std::string directory);
    void ParseDevice(std::string file);

    enum class TokenType 
    {
        Nothing,
        Number,
        String,
        StartBrace,
        EndBrace,
        LineEnd
    };

    struct Token
    {
        Token() : type(TokenType::Nothing), value("") {}
        Token(TokenType _type) : type(_type), value("") {}

        TokenType type;
        std::variant<long, std::string> value;
    };

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
        uint16_t usage_page = 0;
        uint16_t usage = 0;
        std::vector<Collection> subCollections;
        std::vector<ButtonMapping> buttonMappings;            
    };    

    struct Device
    {
        std::string ttyDevice;
        std::string ttyModel;
        std::string manufacturer;
        std::string model;
        Collection baseCollection;
    };

    std::vector<Token> Tokenize(std::string file);
    void FinalizeToken(Token& token);
    Device ParseTokenStream(std::vector<Token> tokens);

    std::vector<Device> m_devices;

    std::string m_directory;
    std::string m_database;
};

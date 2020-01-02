#pragma once


#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/phoenix/bind/bind_member_variable.hpp>
#include <boost/phoenix/bind/bind_member_function.hpp>


#include <string>
#include <vector>
#include <tuple>
#include <functional>


namespace fusion = boost::fusion;
namespace phoenix = boost::phoenix;
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

struct Usage
{
    // initialize to "Braille Display" application collection usage.
    // This is pure laziness to make the display collection look right.
    Usage() {usagePage = 0x41; usageId = 0x01;}
    uint16_t usagePage;
    uint16_t usageId;
};

struct Mapping
{
    Mapping() {ttyGroup = -1; ttyKey = -1;}
    Usage usage;
    int ttyGroup;
    int ttyKey;
    std::string name;
};

struct Collection
{
    Usage usage;
    std::vector<Collection> subCollections;
    std::vector<Mapping> mappings;

    void SetContents(Collection& other)
    {
        subCollections = other.subCollections;
        mappings = other.mappings;
    }
};

struct Display
{
    std::string ttyDriver;
    std::string ttyModel;
    std::string ttyCode;
    std::string manufacturer;
    std::string model;
    Collection baseCollection;

    struct Global
    {
        int id;
        std::string value;
    };

    void SetGlobalValue(Display::Global& value)
    {
        std::string Display::* bindings[] = {
            &Display::ttyDriver, &Display::ttyModel, &Display::ttyCode, &Display::manufacturer, &Display::model
        };

        this->*(bindings[value.id]) = value.value;
    }
};

struct Setting
{
    std::string name;
    std::string value;
};

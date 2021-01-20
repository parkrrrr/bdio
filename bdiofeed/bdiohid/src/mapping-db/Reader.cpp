#include "Reader.h"

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
#include <fstream>

namespace fusion = boost::fusion;
namespace phoenix = boost::phoenix;
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

std::vector<Display> Reader::s_displays;
std::vector<Setting> Reader::s_settings;

    template <typename Iterator, typename SkipType>
    struct DisplayGrammar : qi::grammar<Iterator, SkipType>
    {
        DisplayGrammar() : DisplayGrammar::base_type(m_start, "Display")
        {        
            using namespace qi::labels;

            m_qstring %= qi::lexeme[qi::lit('"') > +(qi::char_ - '"') > '"'];
            m_qstring.name("quoted string");

            m_global = m_globalName[phoenix::bind(&Display::Global::id, _val) = _1] > 
                    m_qstring[phoenix::bind(&Display::Global::value, _val) = _1] >> -qi::lit(';');
            m_global.name("global");

            m_number %= qi::uint_ | (qi::lit("0x") > qi::hex);

            m_usage = m_usageName[phoenix::bind(&Usage::usageId, _val) = _1] | 
                    ((m_usagePageName | m_number)[phoenix::bind(&Usage::usagePage, _val) = _1] > 
                    (m_usageName | m_number)[phoenix::bind(&Usage::usageId, _val) = _1]);

            m_mapping = 
                (
                    qi::int_[phoenix::bind(&Mapping::ttyGroup, _val) = _1] |
                    ('(' > qi::int_[phoenix::bind(&Mapping::ttyGroup, _val) = _1] > ',' > -qi::int_[phoenix::bind(&Mapping::ttyKey, _val) = _1] > ')') 
                ) >
                -m_qstring[phoenix::bind(&Mapping::name, _val) = _1] >
                ';';
            m_mapping.name("mapping");

            m_collection =
                '{' >
                m_collectionContents[phoenix::bind(&Collection::SetContents, _val, _1)] > 
                '}';
            m_collection.name("collection");

            m_collectionContents = *(
                (
                    qi::lit("cells") > (
                        qi::lit('8')[phoenix::bind(&Collection::cells, _val) = 8] | 
                        qi::lit('6')[phoenix::bind(&Collection::cells, _val) = 6]
                    ) > ';'
                ) | 
                (m_usage[_a = _1] >> 
                    (
                        m_collection[
                            phoenix::bind(&Collection::usage, _1) = _a,
                            phoenix::push_back(phoenix::bind(&Collection::subCollections, _val), _1)
                        ] |
                        m_mapping[
                            phoenix::bind(&Mapping::usage, _1) = _a,                                               
                            phoenix::push_back(phoenix::bind(&Collection::mappings, _val), _1)
                        ]
                    )
                )
            );

            m_display %= qi::lit("display") > '{' > m_collectionContents > '}';
            m_display.name("display definition");

            m_displayContents = *(m_global[phoenix::bind(&Display::SetGlobalValue, _val, _1)]) >
                    m_display[phoenix::bind(&Display::baseCollection, _val) = _1];

            m_start = m_displayContents[phoenix::push_back(phoenix::ref(Reader::s_displays), _1)];

            qi::on_error<qi::fail>
            (
                m_start,
                std::cerr
                    << phoenix::val("Error! Expecting ")
                    << _4                               // what failed?
                    << phoenix::val(" here: \"")
                    << phoenix::construct<std::string>(_3, _2)   // iterators to error-pos, end
                    << phoenix::val("\"")
                    << std::endl
            );            

        }    

        struct GlobalName : qi::symbols<char, int>
        {
            GlobalName()
            {
                add
                    ("ttydriver", 0)
                    ("ttymodel", 1)
                    ("ttycode", 2)
                    ("manufacturer", 3)
                    ("model", 4)
                ;
            }
        } m_globalName;

        struct UsagePageName : qi::symbols<char, uint16_t>
        {
            UsagePageName()
            {
                add 
                    ("braille", 0x41)
                    ("button", 0x09)
                ;
            }
        } m_usagePageName;

        struct UsageName : qi::symbols<char, uint16_t>
        {
            UsageName()
            {
                add 
                    // comments like the one below refer to sections in HUTRR-78 
                    // [https://www.usb.org/sites/default/files/hutrr78_-_creation_of_a_braille_display_usage_page_0.pdf]
                    // 20.1 Braille Display Device
                    ("display", 0x1)    // display application collection

                    // 20.2 Braille Cells
                    ("row", 0x2)        // row collection (contains cells, cell count, router collections, router buttons)
                    ("cell8", 0x3)      // 8-dot Braille cell (8 bits)
                    ("cell6", 0x4)      // 6-dot Braille cell (8 bits)
                    ("cellcount", 0x5)  // cell count (8 bits)

                    // 20.3 Routers
                    ("router1", 0xfa)   // router set 1
                    ("router2", 0xfb)   // router set 2
                    ("router3", 0xfc)   // router set 3
                    ("router", 0x100)   // router key (contained in router set, array)

                    // 20.4 Braille Buttons
                    ("buttons", 0x200)  // generic Braille button collection
                    
                    ("dot1", 0x201)     // keyboard dot 1
                    ("dot2", 0x202)     // keyboard dot 2
                    ("dot3", 0x203)     // keyboard dot 3
                    ("dot4", 0x204)     // keyboard dot 4
                    ("dot5", 0x205)     // keyboard dot 5
                    ("dot6", 0x206)     // keyboard dot 6
                    ("dot7", 0x207)     // keyboard dot 7
                    ("dot8", 0x208)     // keyboard dot 8
                    
                    ("space", 0x209)    // keyboard space
                    ("lspace", 0x20a)   // keyboard left space
                    ("rspace", 0x20b)   // keyboard right space
                    
                    ("face", 0x20c)     // face controls collection
                    ("left", 0x20d)     // left controls collection
                    ("right", 0x20e)    // right controls collection
                    ("top", 0x20f)      // top controls collection

                    ("jcenter", 0x210)  // joystick center
                    ("jup", 0x211)      // joystick up
                    ("jdown", 0x212)    // joystick down
                    ("jleft", 0x213)    // joystick left
                    ("jright", 0x214)   // joystick right

                    ("dcenter", 0x215)  // d-pad center
                    ("dup", 0x216)      // d-pad up
                    ("ddown", 0x217)    // d-pad down
                    ("dleft", 0x218)    // d-pad left
                    ("dright" ,0x219)   // d-pad right

                    ("pleft", 0x21a)    // pan left
                    ("pright", 0x21b)   // pan right

                    ("rup", 0x21c)      // rocker up
                    ("rdown", 0x21d)    // rocker down
                    ("rpress", 0x21e)   // rocker press            
                ;
            }
        } m_usageName;

        qi::rule<Iterator, uint16_t, SkipType> m_number;
        qi::rule<Iterator, std::string(), SkipType> m_qstring;
        qi::rule<Iterator, Display::Global(), SkipType> m_global;
        qi::rule<Iterator, uint16_t, SkipType> m_usagePage;
        qi::rule<Iterator, uint16_t, SkipType> m_usageId;
        qi::rule<Iterator, Usage(), SkipType> m_usage;
        qi::rule<Iterator, Collection(), SkipType> m_display;
        qi::rule<Iterator, Collection(), qi::locals<Usage>, SkipType> m_collectionContents;
        qi::rule<Iterator, Collection(), SkipType> m_collection;
        qi::rule<Iterator, Mapping(), SkipType> m_mapping;
        qi::rule<Iterator, Display(), SkipType> m_displayContents;
        qi::rule<Iterator, SkipType> m_start;
    };

    template <typename Iterator, typename SkipType>
    struct SettingsGrammar : qi::grammar<Iterator, SkipType>
    {
        SettingsGrammar() : SettingsGrammar::base_type(m_settings, "Settings")
        {
            using namespace qi::labels;

            m_name %= qi::lexeme[+(qi::char_ - ascii::space - '=' - '}')];
            m_name.name("symbol");

            m_qstring %= qi::lexeme[qi::lit('"') >> +(qi::char_ - '"') >> '"'];
            m_qstring.name("quoted string");

            m_setting %= m_name > '=' > m_qstring > ';';
            m_setting.name("setting");

            m_settings = qi::lit("settings") > '{' >
                +(m_setting[ &SettingAction ]) >
                '}';
            m_settings.name("settings definition");

            qi::on_error<qi::fail>
            (
                m_settings,
                std::cout
                    << phoenix::val("Error! Expecting ")
                    << _4                               // what failed?
                    << phoenix::val(" here: \"")
                    << phoenix::construct<std::string>(_3, _2)   // iterators to error-pos, end
                    << phoenix::val("\"")
                    << std::endl
            );  
        }

        static void SettingAction(std::vector<std::string> val)
        {
            Reader::s_settings.emplace_back(Setting{val[0], val[1]});
        }

        qi::rule<Iterator, std::string(), SkipType> m_name;
        qi::rule<Iterator, std::string(), SkipType> m_qstring;
        qi::rule<Iterator, std::vector<std::string>(), SkipType> m_setting;
        qi::rule<Iterator, SkipType> m_settings;
    };

std::string Reader::StripComments(std::string& text)
{
    enum class State {
        Text, OneSlash, Comment, Quote
    };

    State state = State::Text;

    std::string result;
    result.reserve(text.length());

    for (const auto character : text)
    {
        switch (state)
        {
        case State::OneSlash:
            if (character == '/')
            {
                state = State::Comment;
                break;
            }
            state = State::Text;
            result += '/';
            result += character; 
            break;

        case State::Quote:
            if (character == '"')
            {
                state = State::Text;
            }
            result += character;
            break;

        case State::Comment:
            if (character == '\r' || character == '\n')
            {
                state = State::Text;
                result += character;
            }
            break;

        case State::Text:
            switch (character)
            {
            case '"':
                state = State::Quote;
                result += character;
                break;

            case '#':
                state = State::Comment;
                break;

            case '/':
                state = State::OneSlash;
                break;
            
            default:
                result += character;
                break;
            }
            break;
        }
    }
    return result;
}

bool Reader::ReadSettingsFile(const std::string& filename)
{
    std::string fileContents = ReadFile(filename);

    // The skip parser skips whitespace and comments.
    auto skipParser = ascii::space;

    SettingsGrammar<std::string::iterator, decltype(skipParser)> grammar;
    auto bi = fileContents.begin();
    auto ei = fileContents.end();
    bool result = qi::phrase_parse(bi, ei, grammar, skipParser);
    std::cout << "parsed" << std::endl;
    return (result && (bi == ei));
}

bool Reader::ReadDeviceFile(const std::string& filename)
{
    // The skip parser skips whitespace and comments.
    auto skipParser = ascii::space;

    DisplayGrammar<std::string::iterator, decltype(skipParser)> grammar;
    std::string fileContents = ReadFile(filename);

    auto bi = fileContents.begin();
    auto ei = fileContents.end();
    std::cout << "parsing" << std::endl;
    bool result = qi::phrase_parse(bi, ei, grammar, skipParser);
    std::cout << "parsed" << std::endl;
    return (result && (bi == ei));
}

std::string Reader::ReadFile(const std::string& filename)
{
    std::ifstream file;
    file.open(filename, std::ios::in | std::ios::binary | std::ios::ate);
    auto size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::string result;
    result.resize(size);

    file.read(&result[0], size);

    result += ' ';

    return StripComments(result);
}

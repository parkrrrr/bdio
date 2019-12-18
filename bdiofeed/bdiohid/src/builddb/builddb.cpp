#include "builddb.h"
#include "SQLiteCpp/SQLiteCpp.h"
#include <iostream>
#include <fstream>
#include <functional>

const int BRAILLE_USAGE_PAGE = 0x41;

void Application::Run()
{
    TraverseDirectory(m_directory);
    WriteDatabase();
}

void Application::WriteDatabase()
{
    SQLite::Database database(m_database);
}

void Application::TraverseDirectory(std::string directory)
{
    // write me
}

void Application::ParseDevice(std::string file)
{
    auto tokens = Tokenize(file);
    auto device = ParseTokenStream(tokens);
    m_devices.emplace_back(std::move(device));
}

std::vector<Application::Token> Application::Tokenize(std::string filename)
{
    std::vector<Token> result;

    auto EmitToken = [&result](Token&& token)
    {
        if (token.type != TokenType::Nothing)
        {
            result.emplace_back(std::move(token));
        }
    };

    std::ifstream stream(filename, std::ifstream::in);

    Token currentToken;

    while (!stream.eof())
    {
        char c = 0;
        stream >> c;
        switch (c)
        {
            case 0:
                // error case - probably eof
                break;

            case ';':
                if (currentToken.type == TokenType::String)
                {
                    goto handleCharacter;
                }
                // fall through

            case '\n':
            case '\r':
                // end of line
                FinalizeToken(currentToken);
                EmitToken(std::move(currentToken));
                EmitToken(Token(TokenType::LineEnd));
                currentToken = Token();
                break;

            case '{':
                if (currentToken.type == TokenType::String)
                {
                    goto handleCharacter;
                }

                FinalizeToken(currentToken);
                EmitToken(std::move(currentToken));
                EmitToken(Token(TokenType::StartBrace));
                currentToken = Token();
                break;

            case '}':
                if (currentToken.type == TokenType::String)
                {
                    goto handleCharacter;
                }

                FinalizeToken(currentToken);
                EmitToken(std::move(currentToken));
                EmitToken(Token(TokenType::EndBrace));
                currentToken = Token();
                break;

            case ' ':
            case '\t':
                if (currentToken.type == TokenType::String)
                {
                    goto handleCharacter;
                }
                
                FinalizeToken(currentToken);
                EmitToken(std::move(currentToken));
                currentToken = Token();
                break;

            case '"':
            case '\'':
                if (currentToken.type == TokenType::String)
                {
                    // It's already a string, so this is a closing quote (we don't allow embedded quotes.)
                    EmitToken(std::move(currentToken));
                    currentToken = Token();
                }
                else if (std::get<std::string>(currentToken.value).length())
                {
                    // It's not a string, but it has content. That means we have seen something like xxx". 
                    // Pretend it was really xxx ", so finalize and emit xxx and then start a string.
                    FinalizeToken(currentToken);
                    EmitToken(std::move(currentToken));
                    currentToken = Token(TokenType::String);
                }
                else
                {
                    // It has no content. That's a start quote. Mark it as a string and go on.
                    currentToken.type = TokenType::String;
                }

            default: 
            handleCharacter:
                // it's something else. Accumulate the characters into the string value.
                auto str = std::get_if<std::string>(&currentToken.value);
                if (str)
                {
                    str += c;
                }
                break;                
        }
    }
    FinalizeToken(currentToken);
    EmitToken(std::move(currentToken));

    return result;
}

void Application::FinalizeToken(Application::Token& token)
{
    // Tokens only need finalizing if we haven't figured out what they are yet.
    if (token.type == TokenType::Nothing)
    {
        auto str = std::get<std::string>(token.value);

        char *endPtr = nullptr;
        long numericValue = strtol(str.c_str(), &endPtr, 10);
        if (endPtr && !(*endPtr))
        {
            // it's a valid number in some base. Make it so.
            token.type = TokenType::Number;
            token.value = numericValue;
        }
        else
        {
            token.type = TokenType::String;
        }        
    }
}

Application::Device Application::ParseTokenStream(std::vector<Application::Token> tokens)
{
    Device device;

    ButtonMapping newButton;
    Collection newCollection;
    std::vector<Collection*> collectionStack; 

    enum class State
    {
        Start,                  // Initial section where the display names are declared, between declarations
        TtyDevice,              // String="TtyDevice" seen
        TtyModel,               // String="TtyModel" seen
        Manufacturer,           // String="Manufacturer" seen
        Model,                  // String="Model" seen
        CollectionUsagePage,    // usage page seen at topmost level
        CollectionUsage,        // usage seen at topmost level
        CollectionIdle,         // start of collection or between declarations in collection
        Router,                 // "Router" seen - expecting a group ID
        UsagePage,              // usage page seen inside collection (could be button or subcollection)
        Usage,                  // usage seen inside collection (could be button or subcollection)
        ButtonGroupId,          // group ID seen after button usage
        ButtonKeyId,            // key ID seen after group ID - this might be the end of the button decl, or it might have a name after it, so 
                                //    it's a superset of CollectionIdle      
        Error                   // an error has occurred. 
    };

    using TransitionFunction = std::function<State(Token&)>;

    struct Transition
    {
        State startState;
        // if a token type is not in actions, it's invalid and the transition goes to Error.
        std::vector<std::pair<TokenType, std::variant<TransitionFunction, State>>> actions;
    };

    auto PushButton = [&](Token& token)->State 
    {
            // write me
            return State::Start;
    };

    auto PushCollection = [&](Token& token)->State
    {
            // write me
            return State::Start;
    };

    std::vector<Transition> transitions = 
    {
        {
            State::Start,
            {
                // expecting a number, which is the usage page of a collection, or a string which is either a device name string or the 
                // beginning of a collection.
                {TokenType::LineEnd, State::Start},
                {TokenType::Number, [&](Token& token)->State{newCollection.usage_page = std::get<long>(token.value); return State::CollectionUsagePage;}},
                {TokenType::String, [&](Token& token)->State 
                    {
                        const auto& str = std::get<std::string>(token.value);
                        if (noCaseEquals(str, "TtyDevice")) 
                        {
                            return State::TtyDevice;
                        }

                        if (noCaseEquals(str, "TtyModel")) 
                        {
                            return State::TtyModel;
                        }

                        if (noCaseEquals(str, "Manufacturer")) 
                        {
                            return State::Manufacturer;
                        }

                        if (noCaseEquals(str, "Model")) 
                        {
                            return State::Model;
                        }

                        auto [isPage, isCollection, id] = LookupUsageName(str);
                        if (isPage)
                        {
                            newCollection.usage_page = id; 
                            return State::CollectionUsagePage;
                        }
                        newCollection.usage_page = BRAILLE_USAGE_PAGE; 
                        return State::CollectionUsage;
                    }
                } 
            }
        },
        {
            State::TtyDevice,
            {
                // expecting a string for the brltty device name
                {TokenType::String, [&](Token& token)->State                     
                    {
                        device.ttyDevice = std::get<std::string>(token.value); 
                        return State::Start;
                    }
                }
            }
        },
        {
            State::TtyModel,
            {
                // expecting a string for the brltty model name
                {TokenType::String, [&](Token& token)->State 
                    {
                        device.ttyModel = std::get<std::string>(token.value); 
                        return State::Start;
                    }
                }
            }
        },
        {
            State::Manufacturer,
            {
                // expecting a string for the HID manufacturer name
                {TokenType::String, [&](Token& token)->State 
                    {
                        device.manufacturer = std::get<std::string>(token.value); 
                        return State::Start;
                    }
                }
            }
        },
        {
            State::Model,
            {
                // expecting a string for the HID model name
                {TokenType::String, [&](Token& token)->State 
                    {
                        device.model = std::get<std::string>(token.value); 
                        return State::Start;
                    }
                }
            }
        },
        {
            State::CollectionUsagePage,
            {
                // expecting a collection usage, which is either a number or the string ID of a collection-typed usage.
                {TokenType::Number, [&](Token& token)->State 
                    {
                        newCollection.usage = std::get<long>(token.value); 
                        return State::CollectionUsage;
                    }
                },
                {TokenType::String, [&](Token& token)->State
                    {
                        auto str = std::get<std::string>(token.value);
                        auto [isPage, isCollection, id] = LookupUsageName(str);
                        if (!isPage && isCollection)
                        {
                            newCollection.usage = id; 
                            return State::CollectionUsage;
                        }
                        return State::Error;
                    }
                }
            }
        },
        {
            State::CollectionUsage,
            {
                // The only valid thing here is a start brace
                {TokenType::StartBrace, [&](Token&)->State 
                    {
                        if (collectionStack.empty())
                        {
                            // if we've already parsed a collection into the device, that's an error.
                            if (device.baseCollection.usage) 
                            {
                                return State::Error;
                            }

                            device.baseCollection = std::move(newCollection);
                            collectionStack.push_back(&device.baseCollection);
                        }
                        else
                        {
                            collectionStack.back()->subCollections.emplace_back(std::move(newCollection));
                            collectionStack.push_back(&(collectionStack.back()->subCollections.back()));
                        }
                        return State::CollectionIdle;
                    }
                }
            }
        },
        {
            State::CollectionIdle,
            {                
                // expecting either a collection, a button, a newline, or an end brace. Collection/button can be either usage or usage page
                // usage page -> UsagePage (store in newButton)
                // usage -> CollectionUsage or Usage (store in newButton or newCollection as appropriate)
                {TokenType::String, [&](Token& token)->State
                    {
                        auto str = std::get<std::string>(token.value);
                        auto [isPage, isCollection, id] = LookupUsageName(str);
                        if (isPage)
                        {
                            newButton.usage_page = id; 
                            return State::UsagePage;
                        }
                        else if (isCollection)
                        {
                            newCollection.usage = id; 
                            return State::CollectionUsage;
                        }
                        else 
                        {
                            newButton.usage = id; 
                            return State::Usage;
                        }
                    }
                },
                {TokenType::Number, [&](Token& token)->State 
                    {
                        newButton.usage_page = std::get<long>(token.value); 
                        return State::UsagePage;
                    }
                },
                // newline -> CollectionIdle (blank line)
                {TokenType::LineEnd, State::CollectionIdle},        
                // end brace -> CollectionIdle or Start depending on collection stack depth (end of collection)
                {TokenType::EndBrace, [&](Token&)->State 
                    {
                        collectionStack.pop_back();
                        if (collectionStack.empty())
                        {   
                            return State::Start;
                        }
                        else
                        {
                            return State::CollectionIdle;
                        }
                    }
                }
            }
        },
        {
            State::Router,
            {
                // expecting a group ID, transition to CollectionIdle
                {TokenType::Number, [&](Token& token)->State 
                    {
                        newButton.key_group = std::get<long>(token.value); 
                        return State::CollectionIdle;
                    }
                }             
            }
        },
        {
            State::UsagePage,
            {
                // expecting a usage value, transition to either Usage or CollectionUsage depending on usage type (number -> Usage)
                // if CollectionUsage, move newButton usage page to newCollection
                // write me
            }
        },
        {
            State::Usage,
            {
                // this is either a button or a collection with a numeric usage. 
                // expecting either a start brace or a group ID.
                // start brace -> copy button usage info to collection, push collection, transition to CollectionIdle
                // number -> ButtonGroupId
                // write me
            }
        },
        {
            State::ButtonGroupId,
            {
                // expecting a key ID
                // transition to ButtonKeyId
                // write me
            }
        },
        {
            State::ButtonKeyId,
            {
                // expecting either a name string, a button or collection, a newline, or an end brace.
                // usage page -> push button, transition to UsagePage
                // usage -> push button, transition to Usage or CollectionUsage
                // newline -> push button, transition to CollectionIdle
                // end brace -> CollectionIdle or Start depending on collection stack depth
                // write me
            }
        },        
    };
                   // write me

}

  
int main(int argc, char **argv)
{
    if (argc != 3)
    {
            //    write me
    }

    Application application(directory, database);
    application.Run();

    return 0;
}
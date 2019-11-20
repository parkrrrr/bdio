#pragma once
#include "base.h"
#include "reports.h"

class Gadget
{
public:
    using ListenerToken = int;
    using OutputListener = std::function<void(const ByteString& outputReport)>;

    Gadget(Reports& reports);
    ~Gadget();
    
    void SendInputReport(ByteString inputReport);

    ListenerToken AddOutputListener(OutputListener listener);
    void RemoveOutputListener(ListenerToken token);

private:
    void OpenGadgetDevice();
    void InitializeGadgetDevice();
    void DisableGadgetDevice();

    void WriteConfigFile(std::string name, std::string value)
    {
        auto file = fopen(name.c_str(), "w");
        if (file)
        {
            fwrite(value.c_str(), sizeof(char), value.length() + 1, file);
            fclose(file);
        }            
    }

    void WriteConfigFile(std::string name, ByteString value)
    {
        auto file = fopen(name.c_str(), "w");
        if (file)
        {
            fwrite(value.c_str(), sizeof(uint8_t), value.length(), file);
            fclose(file);
        }            
    }

    void MakeConfigDir(std::string name);
    std::string ReadConfigString(SQLite::Database& database, std::string name);
    void ThreadProcedure();

    ListenerToken m_nextToken = 1;
    std::map<ListenerToken, OutputListener> m_listeners;
    std::thread m_outputThread;
    bool m_terminateOutputThread = false;

    FILE* m_gadgetDevice = nullptr;

    Reports& m_reports;
};
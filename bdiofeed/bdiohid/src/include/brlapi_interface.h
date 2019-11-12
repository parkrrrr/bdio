#pragma once
#include "base.h"
#include <brlapi.h>
#include <string>
#include <functional>
#include <map>
#include <thread>

class BrlApiInterface
{
public:
    using ListenerToken = int;
    using Listener = std::function<void(bool /*pressed*/, uint8_t /*group*/, uint8_t /*key*/)>;

    BrlApiInterface();
    ~BrlApiInterface();

    const std::string& GetDriverName() {return m_driverName;}
    const std::string& GetModelName() {return m_modelName;}
    unsigned int GetCellCount() {return m_cellCount;}

    void WriteCells(ByteString bytes);
    ListenerToken AddListener(Listener listener);
    void RemoveListener(ListenerToken token);

private:
    void ThreadProcedure();

    unsigned int m_cellCount = 0; 
    std::string m_driverName;
    std::string m_modelName;

    ListenerToken m_nextToken = 1;
    std::map<ListenerToken, Listener> m_listeners;
    std::thread m_inputThread;
    bool m_terminateInputThread = false;

    brlapi_fileDescriptor m_connection = 0;
};
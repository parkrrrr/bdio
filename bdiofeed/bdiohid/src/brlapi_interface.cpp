#include "include/brlapi_interface.h"


BrlApiInterface::BrlApiInterface()
{
    m_connection = brlapi_openConnection(nullptr, nullptr);
    if (!m_connection) throw std::exception();

    // brltty supports multiple rows. bdio does not, yet.
    unsigned int y = 0;
    brlapi_getDisplaySize(&m_cellCount, &y);

    char name[BRLAPI_MAXNAMELENGTH];
    brlapi_getDriverName(name, sizeof(name));
    m_driverName = name;

    brlapi_getModelIdentifier(name, sizeof(name));
    m_modelName = name;
	
    int tty = brlapi_enterTtyMode(0, m_driverName.c_str());

    m_inputThread = std::thread([&]{this->ThreadProcedure();});
}

BrlApiInterface::~BrlApiInterface()
{
    if (m_inputThread.joinable())
    {
        m_terminateInputThread = true;
        m_inputThread.join();
    }
    if (m_connection)
    {
        brlapi_leaveTtyMode();
        brlapi_closeConnection();
    }
}

void BrlApiInterface::WriteCells(ByteString bytes)
{
    // pad bytes to at least m_cellCount in length
    if (bytes.length() < m_cellCount)
    {
        bytes.append(m_cellCount - bytes.length(), '\0');
    }

    brlapi_writeDots(bytes.data());
}

BrlApiInterface::ListenerToken BrlApiInterface::AddListener(Listener listener)
{
    m_listeners.insert({m_nextToken, listener});
    return m_nextToken++;
}

void BrlApiInterface::RemoveListener(ListenerToken token)
{
    m_listeners.erase(token);
}

void BrlApiInterface::ThreadProcedure()
{
    while (!m_terminateInputThread)
    {
        brlapi_keyCode_t code = 0;
        if (brlapi_readKeyWithTimeout(c_keyTimeoutMs, &code))
        {
            bool pressed = !!(code >> 63);
            uint8_t group = (code >> 8) & 0xff;
            uint8_t key = code & 0xff;

            for (auto& callback : m_listeners)
            {
                callback.second(pressed, group, key);
            }
        }
    }
}
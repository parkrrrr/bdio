#include "gadget.h"
#include "sys/stat.h"
#include "sys/select.h"
#include <string>
#include "unistd.h"
   
Gadget::Gadget(Reports& reports) : m_reports(reports)
{
    InitializeGadgetDevice();
    OpenGadgetDevice();
    m_outputThread = std::thread([&]{this->ThreadProcedure();});
}

Gadget::~Gadget()
{
    if (m_outputThread.joinable())
    {
        m_terminateOutputThread = true;
        m_outputThread.join();
    }

    if (m_gadgetDevice)
    {
        fclose(m_gadgetDevice);
    }

    DisableGadgetDevice();
}

void Gadget::SendInputReport(ByteString inputReport)
{
    fwrite(inputReport.c_str(), sizeof(uint8_t), inputReport.length(), m_gadgetDevice);
    printf("Sent input report of length %d\n", inputReport.length());
}

Gadget::ListenerToken Gadget::AddOutputListener(Gadget::OutputListener listener)
{
    m_listeners.insert({m_nextToken, listener});
    return m_nextToken++;
}

void Gadget::RemoveOutputListener(ListenerToken token)
{
    m_listeners.erase(token);
}

void Gadget::DisableGadgetDevice()
{
    // open database
    SQLite::Database database(c_databaseName);

    // read configs from 'settings' table
    auto configDir = ReadConfigString(database, "configdir");
    WriteConfigFile(configDir + "/g1/UDC", "");
}

void Gadget::InitializeGadgetDevice()
{
    // open database
    SQLite::Database database(c_databaseName);

    // read configs from 'settings' table

    auto udc = ReadConfigString(database, "udc");
    auto configDir = ReadConfigString(database, "configdir");
    auto vid = ReadConfigString(database, "vid");
    auto pid = ReadConfigString(database, "pid");
    auto serial = ReadConfigString(database, "serial");

    std::string gadgetDir = configDir + "/g1";
    MakeConfigDir(gadgetDir);

    WriteConfigFile(gadgetDir + "/idVendor", vid);
    WriteConfigFile(gadgetDir + "/idProduct", pid);

    WriteConfigFile(gadgetDir + "/bcdDevice", "0x0100");
    WriteConfigFile(gadgetDir + "/bcdUSB", "0x0200");
    WriteConfigFile(gadgetDir + "/bDeviceClass", "0xEF");
    WriteConfigFile(gadgetDir + "/bDeviceSubClass", "0x02");
    WriteConfigFile(gadgetDir + "/bDeviceProtocol", "0x01");

    std::string gadgetStrings = gadgetDir + "/strings";
    MakeConfigDir(gadgetStrings);

    std::string englishStrings = gadgetStrings + "/0x409";
    MakeConfigDir(englishStrings);

    WriteConfigFile(englishStrings + "/manufacturer", m_reports.GetManufacturerName());
    WriteConfigFile(englishStrings + "/product", m_reports.GetModelname());
    WriteConfigFile(englishStrings + "/serialnumber", serial);

    std::string configs = gadgetDir + "/configs";
    MakeConfigDir(configs);

    std::string configDir1 = configs + "/c.1";
    MakeConfigDir(configDir1);

    WriteConfigFile(configDir1 + "/MaxPower", "500");

    std::string functions = gadgetDir + "/functions";
    MakeConfigDir(functions);

    std::string functionHid = functions + "/hid.a";
    MakeConfigDir(functionHid);

    WriteConfigFile(functionHid + "/report_desc", m_reports.GetReportDescriptor());
    WriteConfigFile(functionHid + "/report_length", std::to_string(m_reports.GetMaxReportSize()));
    
    symlink(functionHid.c_str(), (configDir1 + "/hid.a").c_str());

    WriteConfigFile(gadgetDir + "/UDC", udc);
}

void Gadget::OpenGadgetDevice()
{
    // open database
    SQLite::Database database(c_databaseName);

    // read configs from 'settings' table
    auto devname = ReadConfigString(database, "devname");
    m_gadgetDevice = fopen(devname.c_str(), "r+");
}

std::string Gadget::ReadConfigString(SQLite::Database& database, std::string name)
{
    SQLite::Statement query(database, "SELECT setting_value FROM settings WHERE setting_name=?;");
    query.bind(1, name.c_str());

    if (query.executeStep())
    {
        return query.getColumn(0).getString();
    }

    return "";
}

void Gadget::MakeConfigDir(std::string name)
{
    mkdir(name.c_str(), 0755);
}

void Gadget::ThreadProcedure()
{
    fd_set rfds;
    auto fd = fileno(m_gadgetDevice);
    while (!m_terminateOutputThread)
    {
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);

        timeval timeVal{1,0}; // 1 sec timeout

        auto retval = select(fd + 1, &rfds, NULL, NULL, &timeVal);
        if (retval == -1 && errno == EINTR)
        {
            continue;
        }
        if (retval < 0) {
            m_terminateOutputThread = true;
        }
        if (FD_ISSET(fd, &rfds)) {
            ByteString outputReport;
            outputReport.resize(m_reports.GetMaxReportSize());
            fread(outputReport.data(), sizeof(uint8_t), outputReport.length(), m_gadgetDevice);

printf("Saw output report of length %d\n", outputReport.length());

            for (auto& callback : m_listeners)
            {
                callback.second(outputReport);
            }
        }        
    }
}
#pragma once
#include "base.h"
#include "reports.h"

class Gadget
{
    Gadget(Reports& reports) : m_reports(reports)
    {
        InitializeGadgetDevice();
    }

    void SendInputReport(ByteString inputReport);

    ListenerToken AddOutputListener(OutputListener listener);
    void RemoveOutputListener(ListenerToken token);

private:
    void InitializeGadgetDevice();
    void ThreadProcedure();

    Reports& m_reports;
};
#include <brlapi.h>
#include <stdio.h>
#include <unistd.h>
#include "base.h"
#include "brlapi_interface.h"
#include "reports.h"
#include "gadget.h"

const char* c_databaseName = "/etc/bdio/bdio.db";

int main()
{
  BrlApiInterface interface;

  printf( "Driver: [%s]\n", interface.GetDriverName().c_str());
  printf( " Model: [%s]\n", interface.GetModelName().c_str());

  auto cellCount = interface.GetCellCount();
  printf( " Cells: [%d]\n", cellCount);

  ByteString cells;
  interface.WriteCells(cells);

  Reports reports(interface);
  Gadget gadget(reports);

  auto token = interface.AddListener(
    [&](bool pressed, uint8_t group, uint8_t key)
    {
      printf( "Key %s : group %d key %d\n", pressed ? "pressed" : "released", group, key);
      reports.SetButton(pressed, group, key);
      gadget.SendInputReport(reports.GetInputReport());
    }
  );

  auto gadgetToken = gadget.AddOutputListener(
    [&](const ByteString& outputReport)
    {
      reports.SetOutputReport(outputReport);
    }
  );

  sleep(60);

  interface.RemoveListener(token);
  gadget.RemoveOutputListener(gadgetToken);

  cells.clear();
  interface.WriteCells(cells);
 
}

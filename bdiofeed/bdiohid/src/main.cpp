#include <brlapi.h>
#include <stdio.h>
#include <unistd.h>
#include "brlapi_interface.h"

int main()
{
  BrlApiInterface interface;

  printf( "Driver: [%s]\n", interface.GetDriverName().c_str());
  printf( " Model: [%s]\n", interface.GetModelName().c_str());

  auto cellCount = interface.GetCellCount();
  printf( " Cells: [%d]\n", cellCount);

  BrlApiInterface::ByteString cells;
  for (unsigned int i = 1; i <= cellCount; ++i)
  {
    cells.append(1, static_cast<uint8_t>(i));
  }

  interface.WriteCells(cells);

  auto token = interface.AddListener(
    [](bool pressed, uint8_t group, uint8_t key)
    {
      printf( "Key %s : group %d key %d\n", pressed ? "pressed" : "released", group, key);
    }
  );

  sleep(60);

  interface.RemoveListener(token);

  cells.clear();
  interface.WriteCells(cells);
 
}

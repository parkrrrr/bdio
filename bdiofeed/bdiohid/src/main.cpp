#include "brlapi.h"
#include "stdio.h"

int main()
{
  auto connection = brlapi_openConnection(nullptr, nullptr);
  if (connection)
  {
    unsigned int x = 0;
    unsigned int y = 0;
    int gds_ret = brlapi_getDisplaySize(&x, &y);

    printf("display has %d %d %d dots.\n", gds_ret, x, y);

    char name[BRLAPI_MAXNAMELENGTH];
    brlapi_getDriverName(name, sizeof(name));
    printf("driver name [%s]\n", name);

    brlapi_getModelIdentifier(name, sizeof(name));
    printf("model name [%s]\n", name);
	
    int tty = brlapi_enterTtyMode(0, "hm");
    printf("tty %d\n", tty);
    
    unsigned char dots[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32};
    brlapi_writeDots(dots);

    brlapi_keyCode_t code;
    while (1 == brlapi_readKey(1, &code))
    {
      printf("keypress %d\n", (int)code );
    }

    brlapi_leaveTtyMode(); 

    brlapi_closeConnection();
  }

  return 0;
}

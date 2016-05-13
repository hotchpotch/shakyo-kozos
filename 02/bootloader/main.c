#include "defines.h"
#include "serial.h"
#include "lib.h"

int main(void)
{
  serial_init(SERIAL_DEFAULT_DEVICE);
  puts("Hello World!\n");

  putxval(0x10, 0);
  puts("\n");
  putxval(0x10, 2);
  puts("\n");
  putxval(0xffff, 2);
  puts("\n");
  putxval(0xffff, 6);
  puts("\n");
  while (1)
    ;

  return 0;
}


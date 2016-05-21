#include "defines.h"
#include "kozos.h"
#include "interrupt.h"
#include "lib.h"

static int start_threads(int argc, char *argv[])
{
  kz_run(test10_1_main, "test10_1", 1, 0x000, 0, NULL);

  kz_chpri(15);
  INTR_ENABLE;

  while(1) {
    asm volatile("sleep");
  }

  return 0;
}

int main(void)
{
  INTR_DISABLE;

  puts("kozos boot succeed!\n");

  kz_start(start_threads, "idle", 0, 0x100, 0, NULL);

  return 0;
}

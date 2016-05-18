# 1 "interrupt.c"
# 1 "<built-in>"
# 1 "<command line>"
# 1 "interrupt.c"
# 1 "defines.h" 1






typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;
# 2 "interrupt.c" 2
# 1 "intr.h" 1
# 3 "interrupt.c" 2
# 1 "interrupt.h" 1




extern char softvec;


typedef short softvec_type_t;

typedef void (*softvec_handler_t)(softvec_type_t type, unsigned long sp);






int softvec_init(void);

int softvec_setintr(softvec_type_t type, softvec_handler_t handler);

void interrupt(softvec_type_t type, unsigned long sp);
# 4 "interrupt.c" 2

int softvec_init(void)
{
  int type;
  for (type = 0; type < 3; type++)
    softvec_setintr(type, ((void *)0));
  return 0;
}

int softvec_setintr(softvec_type_t type, softvec_handler_t handler)
{
  ((softvec_handler_t *)(&softvec))[type] = handler;
  return 0;
}

void interrupt(softvec_type_t type, unsigned long sp)
{
  softvec_handler_t handler = ((softvec_handler_t *)(&softvec))[type];
  if (handler)
    handler(type, sp);
}

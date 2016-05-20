
#include "defines.h"
#include "kozos.h"
#include "intr.h"
#include "interrupt.h"
#include "syscall.h"
#include "lib.h"

#define THREAD_NUM 6
#define THREAD_NAME_SIZE 15

typedef struct _kz_context {
  uint32 sp;
} kz_context;

typedef struct _kz_thread {
  struct _kz_thread *next;
  char name[THREAD_NAME_SIZE + 1];
  char *stack;

  struct {
    kz_func_t func;
    int argc;
    char **argv;
  } init;

  struct {
    kz_syscall_type_t type;
    kz_syscall_param_t *param;
  } syscall;

  kz_context context;
} kz_thread;

static struct {
  kz_thread *head;
  kz_thread *tail;
} readyque;

static kz_thread *current;
static kz_thread threads[THREAD_NUM];
static kz_handler_t handlers[SOFTVEC_TYPE_NUM];

void dispatch(kz_context *context);

static int getcurrent(void)
{
  if (current == NULL) {
    return -1;
  }

  readyque.head = current->next;
  if (readyque.head == NULL) {
    readyque.tail = NULL;
  }
  current->next = NULL;

  return 0;
}

static int putcurrent(void)
{
  if (current == NULL) {
    return -1;
  }

  if (readyque.tail) {
    readyque.tail->next = current;
  } else {
    readyque.head = current;
  }
  readyque.tail = current;

  return 0;
}

static void thread_end(void)
{
  kz_exit();
}

static void thread_init(kz_thread *thp)
{
  thp->init.func(thp->init.argc, thp->init.argv);
  thread_end();
}

static kz_thread_id_t thread_run(kz_func_t func, char *name,
    int stacksize, int argc, char *argv[])
{
  if (DEBUG) {
    puts("thread run-> ");
    puts(name);
    puts("\n");
  }

  int i;
  kz_thread *thp;
  uint32 *sp;
  extern char userstack;
  static char *thread_stack = &userstack;

  for (i = 0; i < THREAD_NUM; i++) {
    thp = &threads[i];
    if (!thp->init.func)
      break;
  }

  if (i == THREAD_NUM)
    return -1;

  memset(thp, 0, sizeof(*thp));

  strcpy(thp->name, name);
  thp->next = NULL;
  thp->init.func = func;
  thp->init.argc = argc;
  thp->init.argv = argv;

  memset(thread_stack, 0, stacksize);
  thread_stack += stacksize;

  thp->stack = thread_stack;

  // init stack
  sp = (uint32 *)thp->stack;
  *(--sp) = (uint32)thread_end;

  // set program counter
  *(--sp) = (uint32)thread_init;

  *(--sp) = 0; /* ER6 */
  *(--sp) = 0; /* ER5 */
  *(--sp) = 0; /* ER4 */
  *(--sp) = 0; /* ER3 */
  *(--sp) = 0; /* ER2 */
  *(--sp) = 0; /* ER1 */

  *(--sp) = (uint32)thp; /* ER0, first arg */

  thp->context.sp = (uint32)sp;

  putcurrent();
  current = thp;
  putcurrent();

  return (kz_thread_id_t)current;
}

static int thread_exit(void)
{
  puts(current->name);
  puts(" EXIT.\n");
  memset(current, 0, sizeof(*current));
  return 0;
}

static int setintr(softvec_type_t type, kz_handler_t handler)
{
  static void thread_intr(softvec_type_t type, unsigned long sp);

  softvec_setintr(type, thread_intr);
  handlers[type] = handler;

  return 0;
}

static void call_functions(kz_syscall_type_t type, kz_syscall_param_t *p)
{
  switch(type) {
    case KZ_SYSCALL_TYPE_RUN:
      p->un.run.ret = thread_run(p->un.run.func, p->un.run.name,
          p->un.run.stacksize,
          p->un.run.argc, p->un.run.argv);
      break;
    case KZ_SYSCALL_TYPE_EXIT:
     thread_exit();
    break;
  default:
    break; 
  }
}

static void kz_syscall_proc(kz_syscall_type_t type, kz_syscall_param_t *p)
{
  getcurrent();
  call_functions(type, p);
}

static void schedule(void)
{
  if (!readyque.head)
    kz_sysdown();

  current = readyque.head;
}

static void syscall_intr(void)
{
  kz_syscall_proc(current->syscall.type, current->syscall.param);
}

static void softerr_intr(void)
{
  puts(current->name);
  puts(" DOWN.\n");
  getcurrent();
  thread_exit();
}

static void thread_intr(softvec_type_t type, unsigned long sp)
{
  current->context.sp = sp;
  if(handlers[type])
    handlers[type]();

  schedule();

  dispatch(&current->context);
}

void kz_start(kz_func_t func, char *name, int stacksize,
    int argc, char *argv[])
{
  current = NULL;

  readyque.head = readyque.tail = NULL;
  memset(threads, 0, sizeof(threads));
  memset(handlers, 0, sizeof(handlers));

  setintr(SOFTVEC_TYPE_SYSCALL, syscall_intr);
  setintr(SOFTVEC_TYPE_SOFTERR, softerr_intr);

  current = (kz_thread *)thread_run(func, name, stacksize, argc, argv);

  dispatch(&current->context);
}

void kz_sysdown(void)
{
  puts("system error!\n");
  while(1)
    ;
}

void kz_syscall(kz_syscall_type_t type, kz_syscall_param_t *param)
{
  current->syscall.type = type;
  current->syscall.param = param;
  asm volatile ("trapa #0");
}

int test08_1_main(int argc, char *argv[]);


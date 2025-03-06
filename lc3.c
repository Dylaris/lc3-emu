#include "lc3.h"
#include <string.h>

lc3 *new_vm(const char *prog, size_t size)
{
    lc3 *vm = malloc(RAM_SIZE*2);
    if (!vm) ERR("vm malloc");

    memset(vm->ram, 0, RAM_SIZE*2);
    memcpy(vm->ram + PROG_LOAD_ADDR, prog, size);
    for (int i = 0; i < 8; i++) vm->r[i] = 0;
    vm->pc = PROG_LOAD_ADDR;
    vm->psr = 0x8001;
    vm->sp = &vm->r[6];

    return vm;
}
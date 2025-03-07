#include "lc3.h"
#include <string.h>

#define BIT(i, n)      (((i) >> (n)) & 0x1)                                // i[n]
#define FIELD(i, h, l) (((i) >> (l)) & ((1U << ((h) - (l) + 1)) - 1))      // i[h:l]
#define SEXT(i5)       (((i5) & 0x10) ? ((i5) | 0xFFE0) : ((i5) & 0x001F)) // sign extend

#define NEGATIVE 0xFFFC // psr[2:0] = 100
#define ZERO     0xFFFA // psr[2:0] = 010
#define POSIVITE 0xFFF9 // psr[2:0] = 001

#define OP_ADD 0x1 // 0001
#define OP_AND 0x5 // 0101

lc3 *vm_new(const char *prog, size_t size)
{
    lc3 *vm = malloc(sizeof(lc3));
    if (!vm) ERR("vm malloc");

    memset(vm->ram, 0, sizeof(vm->ram));
    memcpy(vm->ram + PROG_LOAD_ADDR, prog, size);
    for (int i = 0; i < 8; i++) vm->r[i] = 0;
    vm->_size = (size - 1) / 2 + 1;
    vm->pc = PROG_LOAD_ADDR;
    vm->psr = 0x8001;
    vm->sp = &vm->r[6];

    return vm;
}

/* set condition codes */
static void setcc(lc3 *vm, u16 result)
{
    if (BIT(result, 15)) 
        vm->psr = (vm->psr & 0xFFF8) & NEGATIVE;
    else if (result == 0)
        vm->psr = (vm->psr & 0xFFF8) & ZERO;
    else 
        vm->psr = (vm->psr & 0xFFF8) & POSIVITE;
}

static void op_add(lc3 *vm, u16 inst)
{
    u16 *dr  = &vm->r[FIELD(inst, 11, 9)];
    u16 *sr1 = &vm->r[FIELD(inst, 8, 6)];

    if (BIT(inst, 5) == 0) {
        u16 *sr2 = &vm->r[FIELD(inst, 2, 0)];
        *dr = *sr1 + *sr2;
    } else {
        u16 imm5 = FIELD(inst, 4, 0);
        *dr = *sr1 + SEXT(imm5);
    }
    setcc(vm, *dr);
}

static void op_and(lc3 *vm, u16 inst)
{
    u16 *dr  = &vm->r[FIELD(inst, 11, 9)];
    u16 *sr1 = &vm->r[FIELD(inst, 8, 6)];

    if (BIT(inst, 5) == 0) {
        u16 *sr2 = &vm->r[FIELD(inst, 2, 0)];
        *dr = *sr1 & *sr2;
    } else {
        u16 imm5 = FIELD(inst, 4, 0);
        *dr = *sr1 & SEXT(imm5);
    }
    setcc(vm, *dr);
}

int vm_exec(lc3 *vm)
{
    if (vm->pc >= vm->_size + PROG_LOAD_ADDR)
        return EXEC_END;

    u16 inst = vm->ram[vm->pc++]; 
    u8 opcode = FIELD(inst, 15, 12);

    switch (opcode) {
    case OP_ADD: op_add(vm, inst); break;
    case OP_AND: op_and(vm, inst); break;
    }

    return EXEC_OK;
}
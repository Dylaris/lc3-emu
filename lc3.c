#include <string.h>
#include "lc3.h"

#define BIT(i, n)      (((i) >> (n)) & 1)                             // i[n]
#define FIELD(i, h, l) (((i) >> (l)) & ((1U << ((h) - (l) + 1)) - 1)) // i[h:l]
#define SEXT(i, n)     (((i) & (1 << ((n) - 1))) ? ((i) | (0xFFFF << (n))) : ((i) & ((1 << (n)) - 1))) // sign extend
#define ZEXT(i, n)     ((i) & ((1 << (n)) - 1)) // zero extend

#define NEGATIVE 0xFFFC // psr[2:0] = 100
#define ZERO     0xFFFA // psr[2:0] = 010
#define POSIVITE 0xFFF9 // psr[2:0] = 001

#define OP_BR   0x0 // 0000
#define OP_ADD  0x1 // 0001
#define OP_LD   0X2 // 0010
#define OP_ST   0x3 // 0011
#define OP_JSR  0x4 // 0100
#define OP_JSRR 0x4 // 0100
#define OP_AND  0x5 // 0101
#define OP_LDR  0x6 // 0110
#define OP_STR  0x7 // 0111
#define OP_RTI  0x8 // 1000
#define OP_NOT  0x9 // 1001
#define OP_LDI  0xA // 1010
#define OP_STI  0xB // 1011
#define OP_JMP  0xC // 1100
#define OP_RET  0xC // 1100
#define OP_LEA  0xE // 1110
#define OP_TRAP 0xF // 1111

lc3 *vm_new(const char *prog, size_t size)
{
    lc3 *vm = malloc(sizeof(lc3));
    if (!vm) ERR("vm malloc");

    memset(vm->ram, 0, sizeof(vm->ram));
    memcpy(vm->ram + PROG_LOAD_ADDR, prog, size);
    for (int i = 0; i < 8; i++) vm->r[i] = 0;
    vm->_size = (size - 1) / 2 + 1;
    vm->pc    = PROG_LOAD_ADDR;
    vm->psr   = 0x8001;
    vm->sp    = &vm->r[6];
    vm->kbsr  = &vm->ram[KBSR_OFF];
    vm->kbdr  = &vm->ram[KBDR_OFF];
    vm->dsr   = &vm->ram[DSR_OFF];
    vm->ddr   = &vm->ram[DDR_OFF];

    return vm;
}

/* set condition codes */
static void setcc(lc3 *vm, u16 result)
{
    if (BIT(result, 15)) 
        vm->psr = vm->psr & NEGATIVE;
    else if (result == 0)
        vm->psr = vm->psr & ZERO;
    else 
        vm->psr = vm->psr & POSIVITE;
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
        *dr = *sr1 + SEXT(imm5, 5);
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
        *dr = *sr1 & SEXT(imm5, 5);
    }
    setcc(vm, *dr);
}

static void op_br(lc3 *vm, u16 inst)
{
    u16 pc_off = FIELD(inst, 8, 0);
    u8 n = BIT(inst, 11), z = BIT(inst, 10), p = BIT(inst, 9);
    u8 N = BIT(vm->psr, 2), Z = BIT(vm->psr, 1), P = BIT(vm->psr, 0);

    if ((n && N) || (z && Z) || (p && P))
        vm->pc += SEXT(pc_off, 9);
}

static void op_jmp(lc3 *vm, u16 inst)
{
    u16 addr = vm->r[FIELD(inst, 8, 6)];
    vm->pc = addr;
}

static void op_jsr(lc3 *vm, u16 inst)
{
    vm->r[7] = vm->pc;
    if (BIT(inst, 11)) { // jsr
        u16 pc_off = FIELD(inst, 10, 0);
        vm->pc += SEXT(pc_off, 11);
    } else {             // jsrr
        u16 addr = vm->r[FIELD(inst, 8, 6)];
        vm->pc = addr;
    }
}

static void op_ld(lc3 *vm, u16 inst)
{
    u16 *dr = &vm->r[FIELD(inst, 11, 9)];
    u16 pc_off = FIELD(inst, 8, 0);
    u16 addr = vm->pc + SEXT(pc_off, 9);
    *dr = vm->ram[addr];
    setcc(vm, addr);
}

static void op_ldi(lc3 *vm, u16 inst)
{
    u16 *dr = &vm->r[FIELD(inst, 11, 9)];
    u16 pc_off = FIELD(inst, 8, 0);
    u16 addr1 = vm->pc + SEXT(pc_off, 9);
    u16 addr2 = vm->ram[addr1];
    *dr = vm->ram[addr2];
    setcc(vm, addr1);
}

static void op_ldr(lc3 *vm, u16 inst)
{
    u16 *dr = &vm->r[FIELD(inst, 11, 9)];
    u16 off = FIELD(inst, 5, 0);
    u16 baser = vm->r[FIELD(inst, 8, 6)];
    u16 addr = baser + SEXT(off, 6);
    *dr = vm->ram[addr];
    setcc(vm, addr);
}

static void op_lea(lc3 *vm, u16 inst)
{
    u16 *dr = &vm->r[FIELD(inst, 11, 9)];
    u16 pc_off = FIELD(inst, 8, 0);
    *dr= vm->pc + SEXT(pc_off, 9);
    setcc(vm, *dr);
}

static void op_not(lc3 *vm, u16 inst)
{
    u16 sr  =  vm->r[FIELD(inst, 8, 6)];
    u16 *dr = &vm->r[FIELD(inst, 11, 9)];
    *dr = ~sr;
    setcc(vm, *dr);
}

static void op_rti(lc3 *vm, u16)
{
    if (BIT(vm->psr, 15)) // user mode
        ERR("error: rti (a privilege mode violation exception)");

    // supervisor mode
    vm->pc = *vm->sp;
    (*vm->sp)++;
    u16 tmp = *vm->sp;
    (*vm->sp)++;
    vm->psr = tmp;
}

static void op_st(lc3 *vm, u16 inst)
{
    u16 sr = vm->r[FIELD(inst, 11, 9)];
    u16 pc_off = FIELD(inst, 8, 0);
    u16 addr = vm->pc + SEXT(pc_off, 9);
    vm->ram[addr] = sr;
}

static void op_sti(lc3 *vm, u16 inst)
{
    u16 sr = vm->r[FIELD(inst, 11, 9)];
    u16 pc_off = FIELD(inst, 8, 0);
    u16 addr1 = vm->pc + SEXT(pc_off, 9);
    u16 addr2 = vm->ram[addr1];
    vm->ram[addr2] = sr;
}

static void op_str(lc3 *vm, u16 inst)
{
    u16 sr = vm->r[FIELD(inst, 11, 9)];
    u16 off = FIELD(inst, 5, 0);
    u16 baser = vm->r[FIELD(inst, 8, 6)];
    u16 addr = baser+ SEXT(off, 6);
    vm->ram[addr] = sr;
}

static void op_trap(lc3 *vm, u16 inst)
{
    vm->r[7] = vm->pc;
    u16 trapvect = FIELD(inst, 7, 0);
    u16 addr = ZEXT(trapvect, 8);
    vm->pc = vm->ram[addr];
}

int vm_exec(lc3 *vm)
{
    if (vm->pc >= vm->_size + PROG_LOAD_ADDR)
        return EXEC_END;

    u16 inst = vm->ram[vm->pc++]; 
    u8 opcode = FIELD(inst, 15, 12);

    switch (opcode) {
    case OP_ADD:  op_add(vm, inst);  break;
    case OP_AND:  op_and(vm, inst);  break;
    case OP_BR:   op_br(vm, inst);   break;
    case OP_JMP:  op_jmp(vm, inst);  break; // OP_RET
    case OP_JSR:  op_jsr(vm, inst);  break; // OP_JSRR
    case OP_LD:   op_ld(vm, inst);   break;
    case OP_LDI:  op_ldi(vm, inst);  break;
    case OP_LDR:  op_ldr(vm, inst);  break;
    case OP_LEA:  op_lea(vm, inst);  break;
    case OP_NOT:  op_not(vm, inst);  break;
    case OP_RTI:  op_rti(vm, inst);  break;
    case OP_ST:   op_st(vm, inst);   break;
    case OP_STI:  op_sti(vm, inst);  break;
    case OP_STR:  op_str(vm, inst);  break;
    case OP_TRAP: op_trap(vm, inst); break;
    default: ERR("error: invalid opcode");
    }

    return EXEC_OK;
}

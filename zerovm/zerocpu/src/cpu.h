#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define SIZE_OF_VECTORS 0

typedef void * context_t;
typedef uint8_t *(*mmu_v2r_t)(context_t ctx, uint32_t vaddr);
typedef uint32_t (*mmu_r2v_t)(context_t ctx, uint8_t *raddr);
typedef void (*syscall_t)(context_t ctx, int num, uint8_t *reg);

struct cpu_tag {
    // machine
    context_t ctx;
    mmu_v2r_t mmuV2R;
    mmu_r2v_t mmuR2V;
    syscall_t syscallHook;

    // regs
    uint8_t acc;
    uint8_t psw; // S,Z,i,(H),o,V,N,C
    uint8_t adH;
    uint8_t adL;
    uint8_t mem;
    uint8_t tmr;
    uint8_t pio;
    uint8_t sio;

    uint8_t reg0;
    uint8_t reg1;
    uint8_t reg2;
    uint8_t reg3;
    uint8_t reg4;
    uint8_t reg5;
    uint8_t reg6;
    uint8_t reg7;
    uint8_t reg8;
    uint8_t reg9;
    uint8_t regA;
    uint8_t regB;
    uint8_t regC;
    uint8_t regD;
    uint8_t regE;
    uint8_t regF;

    // reg ptr
    uint8_t *r[32];

    // SI
    bool SI;

    //
    // internal
    //

    // fetch
    uint16_t pc;
    uint16_t addr;
    uint16_t bin;

    // decode, exec
    bool JUMP;
    bool PAGE;
    bool PAGE0;

    uint8_t INS;
    uint8_t G1;
    uint8_t FNC;
    uint8_t G2;
    uint8_t G3;
    uint8_t CNS;

    bool aH;
    bool aL;
    bool memR;
    bool memW;
};
#ifndef _CPU_T_
#define _CPU_T_
typedef struct cpu_tag cpu_t;
#endif

extern char *toRegName[];

#define POS_S (7) // sign
#define POS_Z (6) // zero
#define POS_i (5) // in
#define POS_H (4) // half carry
#define POS_o (3) // out
#define POS_P (2) // parity or overflow
#define POS_V (2) // parity or overflow
#define POS_N (1) // negative
#define POS_C (0) // carry

#define PSW_S (1<<POS_S)
#define PSW_Z (1<<POS_Z)
#define PSW_i (1<<POS_i)
#define PSW_H (1<<POS_H)
#define PSW_o (1<<POS_o)
#define PSW_P (1<<POS_P)
#define PSW_V (1<<POS_V)
#define PSW_N (1<<POS_N)
#define PSW_C (1<<POS_C)

void init(
    cpu_t *pcpu,
    context_t ctx,
    mmu_v2r_t v2r,
    mmu_r2v_t r2v,
    syscall_t syscallHook,
    uint16_t sp, uint16_t pc);

uint16_t getSP(cpu_t *pcpu);
uint16_t getPC(cpu_t *pcpu);

uint16_t fetch(cpu_t *pcpu);
void decode(cpu_t *pcpu);
void exec(cpu_t *pcpu);

void disasm(cpu_t *pcpu);

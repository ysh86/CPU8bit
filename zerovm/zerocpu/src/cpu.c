#include <stdio.h>
#include <assert.h>

#include "cpu.h"
#include "util.h"

char *toRegName[] = {
    "acc",
    "psw",
    "adH",
    "adL",
    "mem",
    "tmr",
    "pio",
    "sio",

    "ext0",
    "ext1",
    "ext2",
    "ext3",
    "ext4",
    "ext5",
    "ext6",
    "ext7",

    "reg0",
    "reg1",
    "reg2",
    "reg3",
    "reg4",
    "reg5",
    "reg6",
    "reg7",
    "reg8",
    "reg9",
    "regA",
    "regB",
    "regC",
    "regD",
    "regE",
    "regF",
};

void alu(cpu_t *pcpu, uint8_t B2, uint8_t *D, bool S[8]);
void setclear(cpu_t *pcpu, bool S[8]);

void init(
    cpu_t *pcpu,
    context_t ctx,
    mmu_v2r_t v2r,
    mmu_r2v_t r2v,
    syscall_t syscallHook,
    uint16_t sp, uint16_t pc) {
    // machine
    pcpu->ctx = ctx;
    pcpu->mmuV2R = v2r;
    pcpu->mmuR2V = r2v;
    pcpu->syscallHook = syscallHook;

    // set reg ptr
    pcpu->r[0] = &pcpu->acc;
    pcpu->r[1] = &pcpu->psw;
    pcpu->r[2] = &pcpu->adH;
    pcpu->r[3] = &pcpu->adL;
    pcpu->r[4] = &pcpu->mem;
    pcpu->r[5] = &pcpu->tmr;
    pcpu->r[6] = &pcpu->pio;
    pcpu->r[7] = &pcpu->sio;

    pcpu->r[0x10] = &pcpu->reg0;
    pcpu->r[0x11] = &pcpu->reg1;
    pcpu->r[0x12] = &pcpu->reg2;
    pcpu->r[0x13] = &pcpu->reg3;
    pcpu->r[0x14] = &pcpu->reg4;
    pcpu->r[0x15] = &pcpu->reg5;
    pcpu->r[0x16] = &pcpu->reg6;
    pcpu->r[0x17] = &pcpu->reg7;
    pcpu->r[0x18] = &pcpu->reg8;
    pcpu->r[0x19] = &pcpu->reg9;
    pcpu->r[0x1A] = &pcpu->regA;
    pcpu->r[0x1B] = &pcpu->regB;
    pcpu->r[0x1C] = &pcpu->regC;
    pcpu->r[0x1D] = &pcpu->regD;
    pcpu->r[0x1E] = &pcpu->regE;
    pcpu->r[0x1F] = &pcpu->regF;

    // clear regs
    pcpu->acc = 0;
    pcpu->psw = 0;
    pcpu->adH = 0;
    pcpu->adL = 0;
    pcpu->mem = 0;
    pcpu->tmr = 0;
    pcpu->pio = 0;
    pcpu->sio = 0;

    pcpu->reg0 = 0;
    pcpu->reg1 = 0;
    pcpu->reg2 = 0;
    pcpu->reg3 = 0;
    pcpu->reg4 = 0;
    pcpu->reg5 = 0;
    pcpu->reg6 = 0;
    pcpu->reg7 = 0;
    pcpu->reg8 = 0;
    pcpu->reg9 = 0;
    pcpu->regA = 0;
    pcpu->regB = 0;
    pcpu->regC = 0;
    pcpu->regD = 0;
    pcpu->regE = 0;
    pcpu->regF = 0;

    pcpu->SI = false;

    pcpu->pc = pc;
}

uint16_t getSP(cpu_t *pcpu) {
    return 0;
}
uint16_t getPC(cpu_t *pcpu) {
    return pcpu->pc;
}

uint16_t fetch(cpu_t *pcpu) {
    pcpu->addr = pcpu->pc;
    pcpu->bin = read16(false, pcpu->mmuV2R(pcpu->ctx, pcpu->pc + 0x80000000U));
    pcpu->pc += 2;
    return pcpu->bin;
}

void decode(cpu_t *pcpu) {
    pcpu->INS = (pcpu->bin >> 14) & 3;

    // JMP
    bool INS3 = (pcpu->INS == 3);
    bool IF   = (pcpu->bin>>13) & 1;
    bool NT   = (pcpu->bin>>12) & 1;
    uint8_t CNDs = (pcpu->bin>>9) & 7;
    bool AC   = (pcpu->bin>>8) & 1;
    bool PG   = (pcpu->bin>>7) & 1;

    bool cnd  = (pcpu->psw>>CNDs) & 1;
    bool jmp  = (!IF) | (IF & (NT ^ cnd));

    pcpu->JUMPR = INS3 & (AC & !PG);
    pcpu->JUMP  = INS3 & jmp;
    pcpu->PAGE  = pcpu->JUMP & PG;
    pcpu->PAGE0 = !AC;

    pcpu->G1  = (pcpu->bin >>  9) & 31;
    pcpu->FNC = (pcpu->bin >>  5) & 15;
    pcpu->G2  = (pcpu->bin >>  0) & 31;
    pcpu->G3  = (pcpu->bin >> 14) & 1;
    pcpu->CNS = (pcpu->bin >>  0) & 255;

    pcpu->aH   = (pcpu->INS    != 3) & (pcpu->G1 == 2);
    pcpu->aL   = (pcpu->INS    != 3) & (pcpu->G1 == 3);
    pcpu->memR = ((pcpu->INS&2)== 0) & (pcpu->G2 == 4);
    pcpu->memW = (pcpu->INS    != 3) & (pcpu->G1 == 4);
}

void exec(cpu_t *pcpu) {
    // pc (JMP)
    if (pcpu->INS == 3) {
        if (pcpu->JUMPR) {
            pcpu->pc = (*pcpu->r[pcpu->G1]<<8) | ((*pcpu->r[pcpu->G2]<<1) & 255);
        } else if (pcpu->JUMP) {
            uint8_t page = (pcpu->PAGE) ? ((pcpu->PAGE0) ? 0 : pcpu->acc) : (pcpu->pc-2)>>8;
            pcpu->pc = (page<<8) | ((pcpu->bin<<1) & 255);
        }
        return;
    }

    uint8_t B2;
    switch (pcpu->G2) {
    case 0:
        B2 = pcpu->acc;
        break;
    case 1:
        B2 = pcpu->psw;
        break;
    case 2:
        B2 = pcpu->adH;
        break;
    case 3:
        B2 = pcpu->adL;
        break;
    case 4:
        if (pcpu->memR) {
            pcpu->mem = read8(false, pcpu->mmuV2R(pcpu->ctx, (pcpu->adH<<8) | pcpu->adL));
        }
        B2 = pcpu->mem;
        break;
    case 5:
        B2 = pcpu->tmr;
        break;
    case 6:
        B2 = pcpu->pio;
        break;
    case 7:
        if ((pcpu->INS&2) == 0 && pcpu->syscallHook != NULL) {
            pcpu->syscallHook(pcpu->ctx, 0, &pcpu->sio);
        }
        B2 = pcpu->sio;
        break;
    case 0x10:
        B2 = pcpu->reg0;
        break;
    case 0x11:
        B2 = pcpu->reg1;
        break;
    case 0x12:
        B2 = pcpu->reg2;
        break;
    case 0x13:
        B2 = pcpu->reg3;
        break;
    case 0x14:
        B2 = pcpu->reg4;
        break;
    case 0x15:
        B2 = pcpu->reg5;
        break;
    case 0x16:
        B2 = pcpu->reg6;
        break;
    case 0x17:
        B2 = pcpu->reg7;
        break;
    default:
        B2 = 0xff; // invalid
        break;
    }

    uint8_t B1;
    bool S[8] = {false,false,false,false,false,false,false,false};
    if (pcpu->INS == 2) {
        B1 = pcpu->CNS;
    } else {
        uint8_t D;
        alu(pcpu, B2, &D, S);
        S[5] = pcpu->SI;
        S[3] = false; // SO: always empty
        B1 = D;
    }

    if (pcpu->G3) {
        setclear(pcpu, S);
    }

    switch (pcpu->G1) {
    case 0:
        pcpu->acc = B1;
        break;
    case 1:
        if (!pcpu->G3) pcpu->psw = B1;
        break;
    case 2:
        pcpu->adH = B1;
        break;
    case 3:
        pcpu->adL = B1;
        break;
    case 4:
        pcpu->mem = B1;
        if (pcpu->memW) {
            write8(false, pcpu->mmuV2R(pcpu->ctx, (pcpu->adH<<8) | pcpu->adL), pcpu->mem);
        }
        break;
    case 5:
        pcpu->tmr = B1;
        break;
    case 6:
        pcpu->pio = B1;
        break;
    case 7:
        pcpu->sio = B1;
        if (pcpu->syscallHook != NULL) {
            pcpu->syscallHook(pcpu->ctx, 1, &pcpu->sio);
        }
        break;
    case 0x10:
        pcpu->reg0 = B1;
        break;
    case 0x11:
        pcpu->reg1 = B1;
        break;
    case 0x12:
        pcpu->reg2 = B1;
        break;
    case 0x13:
        pcpu->reg3 = B1;
        break;
    case 0x14:
        pcpu->reg4 = B1;
        break;
    case 0x15:
        pcpu->reg5 = B1;
        break;
    case 0x16:
        pcpu->reg6 = B1;
        break;
    case 0x17:
        pcpu->reg7 = B1;
        break;
    default:
        // do nothing
        break;
    }
}
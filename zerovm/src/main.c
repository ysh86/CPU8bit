#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>

#define DEBUG_LOG 0

#include "../zerocpu/src/cpu.h"
#include "machine.h"
#include "serial.h"

#define CONV_PC(_p_) (((_p_)&0xff00) | (((_p_)>>1)&0x7f))
static void dump_regs(uint32_t pc, cpu_t *pcpu);

// serial
void syscallHook(machine_t *pm, int num, uint8_t *reg) {
    serial_io(pm->serial, num, reg);
}

int main(int argc, char *argv[]) {
    //////////////////////////
    // usage
    //////////////////////////
    if (argc < 2) {
        fprintf(stderr, "Usage: zerovm rom\n");
        return EXIT_FAILURE;
    }

    //////////////////////////
    // rom
    //////////////////////////
    mem_t mem;
    FILE *fp;
    fp = fopen(argv[1], "rb");
    int ret = errno;
    if (fp == NULL) {
        fprintf(stderr, "; [ERR] Can't open ROM file \"%s\": %s\n", (const char *)argv[1], strerror(ret));
        return EXIT_FAILURE;
    }
    size_t n = fread(mem.rom, 1, sizeof(mem.rom), fp);
    ret = errno;
    if (n <= 0) {
        fprintf(stderr, "; [ERR] Can't load ROM file \"%s\": %s\n", (const char *)argv[1], strerror(ret));
        return EXIT_FAILURE;
    }
    fclose(fp);

    //////////////////////////
    // cpu
    //////////////////////////
    cpu_t cpu;

    machine_t machine = {
        mem:    &mem,
        cpu:    &cpu,
        serial: serial_new()
    };

    //////////////////////////
    // run
    //////////////////////////
    init(
        &cpu,
        &machine,
        (mmu_v2r_t)mmuV2R,
        (mmu_r2v_t)mmuR2V,
        (syscall_t)syscallHook,
        0, 0);

    const uint32_t eom = 0xfffe;
    while (1) {
        const uint32_t pc = getPC(&cpu);
        if (pc >= eom) {
            break;
        }

        // serial
        serial_tick(machine.serial, &cpu.SI);

        fetch(&cpu);
        decode(&cpu);
#if DEBUG_LOG
        //fprintf(stderr, "; pid %d: ", getpid());
        //disasm(&cpu);
        if (CONV_PC(pc) == 0x021f) {
            dump_regs(pc, &cpu);
            break;
        }
#endif

        exec(&cpu);
    }

    serial_reset(machine.serial);
    machine.serial = NULL;

    // result
    uint32_t pc = getPC(&cpu);
    fprintf(stderr, "\n");
    fprintf(stderr, "; result: pc=%02x%02x, psw=%04x, pio=%04x(%d)\n", pc>>8, (pc>>1)&0x7f, cpu.psw, cpu.pio, cpu.pio);

    return EXIT_SUCCESS;
}

static void dump_regs(uint32_t pc, cpu_t *pcpu) {
    fprintf(stderr, "; pc: %04x\r\n", CONV_PC(pc));
    fprintf(stderr, ";\r\n");
    fprintf(stderr, "; acc=%02x\r\n", pcpu->acc);
    fprintf(stderr, "; psw=%02x\r\n", pcpu->psw);
    fprintf(stderr, "; adH=%02x\r\n", pcpu->adH);
    fprintf(stderr, "; adL=%02x\r\n", pcpu->adL);
    fprintf(stderr, "; mem=%02x\r\n", pcpu->mem);
    fprintf(stderr, "; tmr=%02x\r\n", pcpu->tmr);
    fprintf(stderr, "; pio=%02x\r\n", pcpu->pio);
    fprintf(stderr, "; sio=%02x\r\n", pcpu->sio);
    fprintf(stderr, ";\r\n");
    fprintf(stderr, "; reg0=%02x\r\n", pcpu->reg0);
    fprintf(stderr, "; reg1=%02x\r\n", pcpu->reg1);
    fprintf(stderr, "; reg2=%02x\r\n", pcpu->reg2);
    fprintf(stderr, "; reg3=%02x\r\n", pcpu->reg3);
    fprintf(stderr, "; reg4=%02x\r\n", pcpu->reg4);
    fprintf(stderr, "; reg5=%02x\r\n", pcpu->reg5);
    fprintf(stderr, "; reg6=%02x\r\n", pcpu->reg6);
    fprintf(stderr, "; reg7=%02x\r\n", pcpu->reg7);
    fprintf(stderr, ";\r\n");
    // work & stack
    for (uint32_t a = 0x0400; a < 0x0600; a += 16) {
        fprintf(stderr, "; %04x:", a);
        for (uint32_t b = 0; b < 16; ++b) {
            fprintf(stderr, " %02x", *pcpu->mmuV2R(pcpu->ctx, a+b));
        }
        fprintf(stderr, "\r\n");
        if (a == 0x0500 - 16) {
            fprintf(stderr, "\r\n");
        }
    }
}

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

#if 0
    const uint32_t eom = 72;
#endif
    while (1) {
#if 0
        const uint32_t pc = getPC(&cpu);
        fprintf(stderr, "; pid %d: pc:%04x eom:%04x\n", getpid(), pc>>1, eom);
        if (pc >= eom) {
            break;
        }
#endif

        // serial
        serial_tick(machine.serial, &cpu.SI);

        fetch(&cpu);
        decode(&cpu);
#if DEBUG_LOG
        fprintf(stderr, "; pid %d: ", getpid());
        disasm(&cpu);
#endif

        exec(&cpu);
    }

    // result
    fprintf(stderr, "; result: pc=%04x, psw=%04x, pio=%04x(%d)\n", getPC(&cpu)>>1, cpu.psw, cpu.pio, cpu.pio);

    return EXIT_SUCCESS;
}

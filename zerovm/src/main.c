#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>

#define DEBUG_LOG 0

#include "../zerocpu/src/cpu.h"

// memory
typedef struct _mem_t {
    uint8_t rom[0x10000];
    uint8_t ram[0x10000];
} mem_t;

static mem_t mem;

static uint8_t *mmuV2R(context_t ctx, uint32_t vaddr) {
    mem_t *m = (mem_t *)ctx;

    // rom
    if (vaddr >= 0x80000000U) {
        return &m->rom[vaddr - 0x80000000U];
    }

    // ram
    return &m->ram[vaddr];
}

static uint32_t mmuR2V(context_t ctx, uint8_t *raddr) {
    // not implemented
    //mem_t *m = (mem_t *)ctx;
    return 0;
}

// syscall
void syscallHook(context_t ctx) {
    cpu_t *pcpu = (cpu_t *)ctx;

    // serial out
    if (pcpu->G1 == 7) {
        putc(pcpu->sio, stdout);
        return;
    }
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

    //////////////////////////
    // run
    //////////////////////////
    init(
        &cpu,
        &mem,
        (mmu_v2r_t)mmuV2R,
        (mmu_r2v_t)mmuR2V,
        (syscall_t)syscallHook,
        0, 0);

    const uint32_t eom = 72;
    while (1) {
        const uint32_t pc = getPC(&cpu);
#if 0
        fprintf(stderr, "; pid %d: pc:%04x eom:%04x\n", getpid(), pc>>1, eom);
#endif

        if (pc >= eom) {
            break;
        }

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

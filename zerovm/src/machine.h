#pragma once

#include <stdint.h>

struct mem_tag {
    uint8_t rom[0x10000];
    uint8_t ram[0x10000];
};
#ifndef _MEM_T_
typedef struct mem_tag mem_t;
#define _MEM_T_
#endif

struct cpu_tag;
#ifndef _CPU_T_
#define _CPU_T_
typedef struct cpu_tag cpu_t;
#endif

struct serial_tag;
#ifndef _SERIAL_T_
#define _SERIAL_T_
typedef struct serial_tag serial_t;
#endif

struct machine_tag {
    mem_t    *mem;
    cpu_t    *cpu;
    serial_t *serial;
};
#ifndef _MACHINE_T_
#define _MACHINE_T_
typedef struct machine_tag machine_t;
#endif

static inline uint8_t *mmuV2R(machine_t *pm, uint32_t vaddr) {
    // rom
    if (vaddr >= 0x80000000U) {
        return &pm->mem->rom[vaddr - 0x80000000U];
    }

    // ram
    return &pm->mem->ram[vaddr];
}

static inline uint32_t mmuR2V(machine_t *pm, uint8_t *raddr) {
    // not implemented
    return 0;
}

#pragma once

#include <stdint.h>
#include <stdbool.h>

#define SERIAL_I 0
#define SERIAL_O 1

struct serial_tag;
#ifndef _SERIAL_T_
#define _SERIAL_T_
typedef struct serial_tag serial_t;
#endif

serial_t *serial_new();
void serial_reset();

void serial_io(serial_t *serial, int io, uint8_t *reg);
void serial_tick(serial_t *serial, bool *irq);

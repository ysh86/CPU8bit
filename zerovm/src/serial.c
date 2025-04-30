#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>

#include "serial.h"

enum si_state_type {
    SI_STATE_IDLE,
    SI_STATE_RECV0,
    SI_STATE_RECV1,
    SI_STATE_RECV2,
    SI_STATE_RECV3,
    SI_STATE_RECV4,
    SI_STATE_RECV5,
    SI_STATE_RECV6,
    SI_STATE_RECV7,
    SI_STATE_STOP,
};
typedef enum si_state_type si_state_t;

struct serial_tag {
    si_state_t si_state;
};

static serial_t theSerial;

serial_t *serial_new() {
    theSerial.si_state = SI_STATE_IDLE;
    return &theSerial;
}

void serial_io(serial_t *serial, int io, uint8_t *reg) {
    if (io == SERIAL_I) {
        *reg = getc(stdin);
        return;
    }

    if (io == SERIAL_O) {
        putc(*reg, stdout);
        return;
    }

    return;
}

static bool isSI() {
    int fd = STDIN_FILENO;

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    struct timeval tv = {
        tv_sec: 0,
        tv_usec: 0
    };

    int ret = select(fd + 1, &rfds, NULL, NULL, &tv);
    if (ret < 0) {
        perror("select()");
        return false;
    }
    if (ret == 0) {
        return false;
    }

    return true;
}

void serial_tick(serial_t *serial, bool *irq) {
    switch (serial->si_state) {
        case SI_STATE_IDLE:
            if (isSI()) {
                serial->si_state = SI_STATE_RECV0;
                *irq = true;
            }
            break;
        case SI_STATE_RECV0:
            serial->si_state = SI_STATE_RECV1;
            break;
        case SI_STATE_RECV1:
            serial->si_state = SI_STATE_RECV2;
            break;
        case SI_STATE_RECV2:
            serial->si_state = SI_STATE_RECV3;
            break;
        case SI_STATE_RECV3:
            serial->si_state = SI_STATE_RECV4;
            break;
        case SI_STATE_RECV4:
            serial->si_state = SI_STATE_RECV5;
            break;
        case SI_STATE_RECV5:
            serial->si_state = SI_STATE_RECV6;
            break;
        case SI_STATE_RECV6:
            serial->si_state = SI_STATE_RECV7;
            break;
        case SI_STATE_RECV7:
            serial->si_state = SI_STATE_STOP;
            break;
        case SI_STATE_STOP:
            serial->si_state = SI_STATE_IDLE;
            *irq = false;
            break;
        default:
            break;
    }
}

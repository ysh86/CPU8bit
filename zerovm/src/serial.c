#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>

#include "serial.h"

enum si_state_type {
    SI_STATE_IDLE,
    SI_STATE_START,
    SI_STATE_RECV0,
    SI_STATE_RECV1,
    SI_STATE_RECV2,
    SI_STATE_RECV3,
    SI_STATE_RECV4,
    SI_STATE_RECV5,
    SI_STATE_RECV6,
    SI_STATE_RECV7,
    SI_STATE_STOP0,
    SI_STATE_STOP1,
    SI_STATE_STOP2,
    SI_STATE_STOP3,
};
typedef enum si_state_type si_state_t;

struct serial_tag {
    FILE *in;
    FILE *out;

    int fd_in;
    struct termios savein;

    si_state_t si_state;
};

static serial_t theSerial;

serial_t *serial_new() {
    theSerial.in = stdin;
    theSerial.out = stdout;
    theSerial.fd_in = fileno(stdin);
    theSerial.si_state = SI_STATE_IDLE;

    setvbuf(theSerial.in, NULL, _IONBF, 0);
    setvbuf(theSerial.out, NULL, _IONBF, 0);

    struct termios modin;
    if (tcgetattr(theSerial.fd_in, &theSerial.savein) < 0) {
        perror("tcgetattr(in)");
        return NULL;
    }
    if (tcgetattr(theSerial.fd_in, &modin) < 0) {
        perror("tcgetattr(in)");
        return NULL;
    }

    modin.c_lflag &= ~ICANON;
    modin.c_lflag &= ~ISIG;

    modin.c_lflag &= ~ECHO;
    modin.c_lflag |= ECHOE;

    modin.c_iflag &= ~ICRNL;
    modin.c_oflag &= ~ONLCR;

    if (tcsetattr(theSerial.fd_in, TCSANOW, &modin) < 0) {
        perror("tcsetattr(in)");
        return NULL;
    }

    return &theSerial;
}

void serial_reset(serial_t *serial) {
    if (tcsetattr(serial->fd_in, TCSANOW, &serial->savein) < 0) {
        perror("tcsetattr(in)");
    }
}

void serial_io(serial_t *serial, int io, uint8_t *reg) {
    if (io == SERIAL_I) {
        *reg = fgetc(serial->in);
        return;
    }

    if (io == SERIAL_O) {
        fputc(*reg, serial->out);
        return;
    }

    return;
}

static bool isSI(int fd) {
    fd_set rfds;

    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    struct timeval tv = {
        tv_sec: 0,
        tv_usec: 0
    };

    int ret = select(FD_SETSIZE, &rfds, NULL, NULL, &tv);
    if (ret < 0) {
        perror("select()");
        return false;
    }
    if (ret == 0) {
        return false;
    }

    return FD_ISSET(fd, &rfds);
}

void serial_tick(serial_t *serial, bool *irq) {
    switch (serial->si_state) {
        case SI_STATE_IDLE:
            if (isSI(serial->fd_in)) {
                serial->si_state = SI_STATE_START;
                *irq = true;
            }
            break;
        case SI_STATE_START:
            serial->si_state = SI_STATE_RECV0;
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
            serial->si_state = SI_STATE_STOP0;
            break;
        case SI_STATE_STOP0:
            serial->si_state = SI_STATE_STOP1;
            *irq = false;
            break;
        case SI_STATE_STOP1:
            serial->si_state = SI_STATE_STOP2;
            break;
        case SI_STATE_STOP2:
            serial->si_state = SI_STATE_STOP3;
            break;
        case SI_STATE_STOP3:
            serial->si_state = SI_STATE_IDLE;
            break;
        default:
            break;
    }
}

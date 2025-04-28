#include "cpu.h"

enum func_type {
    MOV,
    INC,
    INCC,
    DEC,
    DECC,
    ADD,
    ADC,
    SUB,
    SBC,
    AND,
    OR,
    XOR,
    RR,
    RL,
    RRC,
    RLC
};
typedef enum func_type func_t;

void setclear(cpu_t *pcpu, bool S[8]) {
    for (int i = 0; i < 8; i++) {
        if (S[i]) {
            pcpu->psw |= (1<<i);
        } else {
            pcpu->psw &= ~(1<<i);
        }
    }
}

static inline bool zero(uint8_t d) {
    return (d == 0);
}

// true:  even
// false: odd
static inline bool pty(uint8_t d) {
    bool d8[8] = {
        (d>>0)&1,
        (d>>1)&1,
        (d>>2)&1,
        (d>>3)&1,
        (d>>4)&1,
        (d>>5)&1,
        (d>>6)&1,
        (d>>7)
    };
    return !(d8[0]^d8[1]^d8[2]^d8[3]^d8[4]^d8[5]^d8[6]^d8[7]);
}

static inline bool ov_one(bool a, bool d) {
    return (a ^ d);
}

static inline bool ov_plus(bool a, bool b, bool d) {
    return (!ov_one(a,b) & ov_one(a,d));
}

static inline bool ov_minus(bool a, bool b, bool d) {
    return (ov_one(a,b) & ov_one(a,d));
}

void alu(cpu_t *pcpu, uint8_t B2, uint8_t *D, bool S[8]) {
    uint8_t A   = pcpu->acc;
    uint8_t B   = B2;
    uint8_t Cin = pcpu->psw & 1;
    func_t  F   = (func_t)pcpu->FNC;

    uint16_t CD;

    switch (F) {
        case MOV:
            *D = B;
            S[POS_S] = (*D)>>7;
            S[POS_Z] = zero(*D);
            S[POS_H] = false;
            S[POS_P] = pty(*D);
            S[POS_N] = false;
            S[POS_C] = false;
            break;

        case INC:
            CD = B + 1;
            *D = CD & 0xff;
            S[POS_S] = (*D)>>7;
            S[POS_Z] = zero(*D);
            S[POS_H] = false; // not implemented
            S[POS_V] = ov_one(B>>7, (*D)>>7);
            S[POS_N] = false;
            S[POS_C] = (CD>>8)&1;
            break;
        case INCC:
            CD = B + Cin;
            *D = CD & 0xff;
            S[POS_S] = (*D)>>7;
            S[POS_Z] = zero(*D);
            S[POS_H] = false; // not implemented
            S[POS_V] = ov_one(B>>7, (*D)>>7);
            S[POS_N] = false;
            S[POS_C] = (CD>>8)&1;
            break;
        case DEC:
            CD = B - 1;
            *D = CD & 0xff;
            S[POS_S] = (*D)>>7;
            S[POS_Z] = zero(*D);
            S[POS_H] = false; // not implemented
            S[POS_V] = ov_one(B>>7, (*D)>>7);
            S[POS_N] = true;
            S[POS_C] = (CD>>8)&1;
            break;
        case DECC:
            CD = B - Cin;
            *D = CD & 0xff;
            S[POS_S] = (*D)>>7;
            S[POS_Z] = zero(*D);
            S[POS_H] = false; // not implemented
            S[POS_V] = ov_one(B>>7, (*D)>>7);
            S[POS_N] = true;
            S[POS_C] = (CD>>8)&1;
            break;
        case ADD:
            CD = A + B;
            *D = CD & 0xff;
            S[POS_S] = (*D)>>7;
            S[POS_Z] = zero(*D);
            S[POS_H] = false; // not implemented
            S[POS_V] = ov_plus(A>>7, B>>7, (*D)>>7);
            S[POS_N] = false;
            S[POS_C] = (CD>>8)&1;
            break;
        case ADC:
            CD = A + B + Cin;
            *D = CD & 0xff;
            S[POS_S] = (*D)>>7;
            S[POS_Z] = zero(*D);
            S[POS_H] = false; // not implemented
            S[POS_V] = ov_plus(A>>7, B>>7, (*D)>>7);
            S[POS_N] = false;
            S[POS_C] = (CD>>8)&1;
            break;
        case SUB:
            CD = A - B;
            *D = CD & 0xff;
            S[POS_S] = (*D)>>7;
            S[POS_Z] = zero(*D);
            S[POS_H] = false; // not implemented
            S[POS_V] = ov_minus(A>>7, B>>7, (*D)>>7);
            S[POS_N] = true;
            S[POS_C] = (CD>>8)&1;
            break;
        case SBC:
            CD = A - B - Cin;
            *D = CD & 0xff;
            S[POS_S] = (*D)>>7;
            S[POS_Z] = zero(*D);
            S[POS_H] = false; // not implemented
            S[POS_V] = ov_minus(A>>7, B>>7, (*D)>>7);
            S[POS_N] = true;
            S[POS_C] = (CD>>8)&1;
            break;

        case AND:
            *D = A & B;
            S[POS_S] = (*D)>>7;
            S[POS_Z] = zero(*D);
            S[POS_H] = false;
            S[POS_P] = pty(*D);
            S[POS_N] = false;
            S[POS_C] = false;
            break;
        case OR:
            *D = A | B;
            S[POS_S] = (*D)>>7;
            S[POS_Z] = zero(*D);
            S[POS_H] = false;
            S[POS_P] = pty(*D);
            S[POS_N] = false;
            S[POS_C] = false;
            break;
        case XOR:
            *D = A ^ B;
            S[POS_S] = (*D)>>7;
            S[POS_Z] = zero(*D);
            S[POS_H] = false;
            S[POS_P] = pty(*D);
            S[POS_N] = false;
            S[POS_C] = false;
            break;

        case RR:
            *D = Cin | (B>>1);
            S[POS_S] = (*D)>>7;
            S[POS_Z] = zero(*D);
            S[POS_H] = false;
            S[POS_P] = pty(*D);
            S[POS_N] = false;
            S[POS_C] = B & 1;
            break;
        case RL:
            *D = (B<<1) | Cin;
            S[POS_S] = (*D)>>7;
            S[POS_Z] = zero(*D);
            S[POS_H] = false;
            S[POS_P] = pty(*D);
            S[POS_N] = false;
            S[POS_C] = B >> 7;
            break;
        case RRC:
            *D = ((B&1)<<7) | (B>>1);
            S[POS_S] = (*D)>>7;
            S[POS_Z] = zero(*D);
            S[POS_H] = false;
            S[POS_P] = pty(*D);
            S[POS_N] = false;
            S[POS_C] = B & 1;
            break;
        case RLC:
            *D = (B<<1) | (B>>7);
            S[POS_S] = (*D)>>7;
            S[POS_Z] = zero(*D);
            S[POS_H] = false;
            S[POS_P] = pty(*D);
            S[POS_N] = false;
            S[POS_C] = B >> 7;
            break;

        default:
            break;
    }
}

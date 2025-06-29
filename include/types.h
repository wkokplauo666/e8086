#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

// 8086 flags
#define CF (1 << 0)
#define PF (1 << 2)
#define AF (1 << 4)
#define ZF (1 << 6)
#define SF (1 << 7)
#define TF (1 << 8)
#define IF (1 << 9)
#define DF (1 << 10)
#define OF (1 << 11)

#define MEM_INIT (1 << 0)
#define CPU_INIT (1 << 1)
#define TRM_UNBF (1 << 2)
#define SIM_DEBG (1 << 3)

#define SR_NONE 20
#define SR_DF   10
#define SR_DS   3
#define SR_SS   2
#define SR_ES   0
#define SR_CS   1

typedef uint16_t simflg_t;

#endif
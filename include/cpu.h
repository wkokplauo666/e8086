#ifndef CPU_H
#define CPU_H

#include "types.h"
#include <ctype.h>
#include <stdint.h>

typedef struct reg_t {
  uint8_t ax[2];
  uint8_t bx[2];
  uint8_t cx[2];
  uint8_t dx[2];

  uint16_t sp;
  uint16_t bp;
  uint16_t si;
  uint16_t di;

  uint16_t ip;
  uint8_t flg[2];

  uint16_t cs;
  uint16_t ds;
  uint16_t ss;
  uint16_t es;
} reg_t;

extern uint8_t *mem;
extern reg_t g_reg;
extern simflg_t g_simflg;
extern uint8_t s_ovrd;

uint8_t *regtor8(uint8_t reg, reg_t *r);
uint16_t *regtor16(uint8_t reg, reg_t *r);
uint16_t *regtos16(uint8_t reg, reg_t *r);

void intrt_h(uint8_t type, reg_t *r, uint8_t *mem, uint16_t ra);
void handleout(uint16_t port, uint16_t data, uint8_t word);
uint16_t handlein(uint16_t port, uint8_t word);

uint8_t parchk16(uint16_t x);
uint8_t parchk8(uint8_t x);

int init();
void step();

void print_dbg(reg_t *r, uint8_t *m);
void dump(const uint8_t *data, uint16_t len, uint32_t offset, uint32_t haddr);

#endif
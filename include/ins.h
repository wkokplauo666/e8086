#ifndef INS_H
#define INS_H

#include <stdint.h>

#include "cpu.h"

extern uint8_t s_ovrd;

void push (reg_t *r, uint8_t *mem);
void pop  (reg_t *r, uint8_t *mem);
void move (reg_t *r, uint8_t *mem);
void inout(reg_t *r, uint8_t *mem);
void xchg (reg_t *r, uint8_t *mem);
void xlat (reg_t *r, uint8_t *mem);
void lea  (reg_t *r, uint8_t *mem);
void lds  (reg_t *r, uint8_t *mem);
void les  (reg_t *r, uint8_t *mem);
void lsahf(reg_t *r, uint8_t *mem);

void add  (reg_t *r, uint8_t *mem);
void adc  (reg_t *r, uint8_t *mem);
void inc  (reg_t *r, uint8_t *mem);
void cmp  (reg_t *r, uint8_t *mem);

void call (reg_t *r, uint8_t *mem);
void intr (reg_t *r, uint8_t *mem);
void jmp  (reg_t *r, uint8_t *mem);
void ret  (reg_t *r, uint8_t *mem);
void jcc  (reg_t *r, uint8_t *mem);
void loop (reg_t *r, uint8_t *mem);

void pctrl(reg_t *r, uint8_t *mem);

#endif
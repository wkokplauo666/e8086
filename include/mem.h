#ifndef MEM_H
#define MEM_H

#include <stdint.h>
#include "cpu.h"

extern uint8_t *mem;

uint32_t ea_calc(uint8_t rm, uint8_t mod, uint16_t disp, reg_t *r, uint8_t seg);
uint32_t a_seg(uint16_t addr, uint8_t seg, reg_t *r);

void set16(uint16_t data, uint32_t addr, uint8_t *m);
uint16_t fetch16(uint32_t addr, uint8_t *m);
void set8(uint8_t data, uint32_t addr, uint8_t *m);
uint8_t fetch8(uint32_t addr, uint8_t *m);


#endif
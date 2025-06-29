#include "mem.h"
#include "types.h"

uint8_t *mem;

extern uint16_t *regtos16(uint8_t reg, reg_t *r);

uint32_t ea_calc(uint8_t rm, uint8_t mod, uint16_t disp, reg_t *r, uint8_t seg) {
  uint32_t ea = 0;

  switch (rm) {
  case 0x00:
    ea += *(uint16_t *)r->bx + r->si;
    break;
  case 0x01:
    ea += *(uint16_t *)r->bx + r->di;
    break;
  case 0x02:
    ea += r->bp + r->si;
    break;
  case 0x03:
    ea += r->bp + r->di;
    break;
  case 0x04:
    ea += r->si;
    break;
  case 0x05:
    ea += r->di;
    break;
  case 0x06:
    ea += r->bp;
    break;
  case 0x07:
    ea += *(uint16_t *)r->bx;
    break;
  default:
    break;
  }

  switch(mod) {
  case 0x01:
    ea += (int16_t)((int8_t)(disp & 0xFF));
    break;
  case 0x2:
    ea += disp;
    break;
  }

  if(mod == 0x00 && rm == 0x06) ea = disp;

  if((rm == 0x02 || rm == 0x03 || rm == 0x06) && seg == SR_DF) {
    ea = (*regtos16(SR_SS, r) << 4) + ea;
  } else if(seg == SR_DF) {
    ea = (*regtos16(SR_DS, r) << 4) + ea;
  } else if(seg == SR_NONE) {} else {
    ea = (*regtos16(seg, r) << 4) + ea;
  }


  return ea;
}

void set16(uint16_t data, uint32_t addr, uint8_t *m) {
  *(uint16_t *)&m[addr] = data;
}

void set8(uint8_t data, uint32_t addr, uint8_t *m) {
  m[addr] = data;
}

uint16_t fetch16(uint32_t addr, uint8_t *m) {
  return *(uint16_t *)&m[addr];
}

uint8_t fetch8(uint32_t addr, uint8_t *m) {
  return m[addr];
}

uint32_t a_seg(uint16_t addr, uint8_t seg, reg_t *r) {
  if(seg == SR_DF) {
    return (r->ds << 4) + addr;
  }
  return ea_calc(0x06, 0x00, addr, r, seg);
}

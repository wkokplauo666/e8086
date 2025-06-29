#include <string.h>
#include <stdio.h>
#include <limits.h>

#include "ins.h"

#include "cpu.h"
#include "mem.h"

#define CURBYTE mem[a_seg(r->ip, SR_CS, r)]
#define CURWORD fetch16(a_seg(r->ip, SR_CS, r), mem)
#define NXTBYTE mem[a_seg(r->ip + 1, SR_CS, r)]
#define NXTWORD fetch16(a_seg(r->ip + 1, SR_CS, r), mem)

void push(reg_t *r, uint8_t *mem) {
  uint8_t opc = mem[a_seg(r->ip, SR_CS, r)];
  if(opc == 0xff) {
    uint8_t f = mem[a_seg(r->ip + 1, SR_CS, r)];
    if(((f >> 3) & 0b111) != 0b110) return;
    uint8_t rm = f & 0b111;
    uint8_t mod = (f >> 6) & 0b11;

    if(mod == 0b11) {
      uint16_t *reg = regtor16(rm, r);
      set16(*reg, a_seg(r->sp - 2, SR_SS, r), mem);
      r->sp -= 2;
      r->ip += 2;
      return;
    } else {
      uint16_t disp = 0;
      if(mod == 1) disp = mem[a_seg(r->ip + 2, SR_CS, r)];
      if((mod == 2) || ((mod == 0) && (rm == 0x06))) disp = fetch16(a_seg(r->ip + 2, SR_CS, r), mem);

      set16(fetch16(ea_calc(rm, mod, disp, r, s_ovrd), mem), a_seg(r->sp - 2, SR_SS, r), mem);
      r->sp -= 2;
      r->ip += 2 + mod;
      if((mod == 0) && (rm == 0x06)) r->ip += 2;
      return;
    }
  }

  if((opc >> 3) == 0b01010) {
    uint16_t *reg = regtor16(opc & 0b111, r);
    set16(*reg, a_seg(r->sp - 2, SR_SS, r), mem);
    r->sp -= 2;
    r->ip += 1;
    return;
  }

  if(((opc & 0b111) == 0b110) && ((opc >> 5) == 0)) {
    uint16_t *reg = regtos16((opc >> 3) & 0b11, r);
    set16(*reg, a_seg(r->sp - 2, SR_SS, r), mem);
    r->sp -= 2;
    r->ip += 1;
    return;
  }
}

void move(reg_t *r, uint8_t *mem) {
  uint8_t opc = mem[a_seg(r->ip, SR_CS, r)];

  if((opc >> 2) == 0b100010) {
    uint8_t f = mem[a_seg(r->ip + 1, SR_CS, r)];
    uint8_t mod = f >> 6;
    uint8_t reg = (f >> 3) & 0b111;
    uint8_t rm = f & 0b111;
    uint8_t w = opc & 1;
    uint8_t d = (opc >> 1) & 1;

    if(mod == 0b11) {
      uint16_t *regs;
      uint16_t *regd;
      if(w) {
        regs = regtor16(rm, r);
        regd = regtor16(reg, r);
      } else {
        regs = (uint16_t *)regtor16(rm, r);
        regd = (uint16_t *)regtor16(reg, r);
      }

      if(d) *regd = *regs;
      else *regs = *regd;
      r->ip += 2;
      return;
    } else {
      uint16_t disp = 0;
      if(mod == 1) disp = mem[a_seg(r->ip + 2, SR_CS, r)];
      if((mod == 2) || ((mod == 0) && (rm == 0x06))) disp = fetch16(a_seg(r->ip + 2, SR_CS, r), mem);
      if(w) {
        uint16_t *regd = regtor16(reg, r);
        uint16_t *src = (uint16_t *)&mem[ea_calc(rm, mod, disp, r, s_ovrd)];

        if(d) *regd = *src;
        else *src = *regd;

      } else {
        uint8_t *regd = regtor8(reg, r);
        uint8_t *src = &mem[ea_calc(rm, mod, disp, r, s_ovrd)];

        if(d) *regd = *src;
        else *src = *regd;
      }

      r->ip += 2 + mod;
      if((mod == 0) && (rm == 0x06)) r->ip += 2;
      return;
    }
  }

  if((opc >> 1) == 0b1100011) {
    uint8_t f = mem[a_seg(r->ip + 1, SR_CS, r)];
    if(((f >> 3) & 0b111) != 0b000) return;
    uint8_t mod = f >> 6;
    uint8_t rm = f & 0b111;
    uint8_t w = opc & 1;
    if(mod == 0b11) {
      if(w) {
        uint16_t *reg = regtor16(rm, r);
        *reg = fetch16(a_seg(r->ip + 2, SR_CS, r), mem);
        r->ip += 4;
        return;
      } else {
        uint8_t *reg = regtor8(rm, r);
        *reg = mem[a_seg(r->ip + 2, SR_CS, r)];
        r->ip += 3;
        return;
      }
    } else {
      uint16_t disp = 0;
      if(mod == 1) disp = mem[a_seg(r->ip + 2, SR_CS, r)];
      if((mod == 2) || ((mod == 0) && (rm == 0x06))) disp = fetch16(a_seg(r->ip + 2, SR_CS, r), mem);
      uint8_t daddr = r->ip + 2 + mod;
      if((mod == 0) && (rm == 0x06)) daddr += 2;
      if(w) {
        uint16_t regd = fetch16(a_seg(daddr, SR_CS, r), mem);
        uint16_t *src = (uint16_t *)&mem[ea_calc(rm, mod, disp, r, s_ovrd)];

        *src = regd;

      } else {
        uint8_t *regd = &mem[a_seg(daddr, SR_CS, r)];
        uint8_t *src = &mem[ea_calc(rm, mod, disp, r, s_ovrd)];

        *src = *regd;
      }

      r->ip += 3 + mod + w;
      if((mod == 0) && (rm == 0x06)) r->ip += 2;
      return;
    }
  }

  if((opc >> 4) == 0b1011) {
    uint8_t w = (opc >> 3) & 1;
    uint8_t reg = opc & 0b111;

    if(w) {
      uint16_t *rg = regtor16(reg, r);
      *rg = fetch16(a_seg(r->ip + 1, SR_CS, r), mem);
      r->ip += 3;
      return;
    } else {
      uint8_t *rg = regtor8(reg, r);
      *rg = mem[a_seg(r->ip + 1, SR_CS, r)];
      r->ip += 2;
      return;
    }
  }

  if((opc >> 2) == 0b101000) {
    uint8_t w = opc & 1;
    uint8_t d = (opc >> 1) & 1;
    uint32_t addr = a_seg(fetch16(a_seg(r->ip + 1, SR_CS, r), mem), s_ovrd, r);
    if(w) {
      if(d) set16(*(uint16_t *)r->ax, addr, mem);
      else *(uint16_t *)r->ax = fetch16(addr, mem);
      r->ip += 3;
      return;
    } else {
      if(d) set8(r->ax[0], addr, mem);
      else r->ax[0] = fetch8(addr, mem);
      r->ip += 3;
      return;
    }
  }

  if((opc >> 2) == 0b100011) {
    uint8_t f = mem[a_seg(r->ip + 1, SR_CS, r)];
    if(((f >> 5) & 1) != 0) return;
    uint8_t d = (opc >> 1) & 1;
    uint8_t mod = f >> 6;
    uint8_t sreg = (f >> 3) & 0b11;
    uint8_t rm = f & 0b111;

    if(mod == 0b11) {
      uint16_t *a = regtor16(rm, r);
      uint16_t *s = regtos16(sreg, r);

      if(d) *s = *a;
      else *a = *s;
      r->ip += 2;
      return;
    } else {
      uint16_t disp = 0;
      if(mod == 0b01) disp = mem[a_seg(r->ip + 2, SR_CS, r)];
      if((mod == 2) || ((mod == 0) && (rm == 0x06))) disp = fetch16(a_seg(r->ip + 2, SR_CS, r), mem);

      uint16_t *a = (uint16_t *)&mem[ea_calc(rm, mod, disp, r, s_ovrd)];
      uint16_t *s = regtos16(sreg, r);

      if(d) *s = *a;
      else *a = *s;
      r->ip += 2 + mod;
      if((mod == 0x00) && (rm == 0x06)) r->ip += 2;
      return;
    }

  }
}

void pop(reg_t *r, uint8_t *mem) {
  uint8_t opc = mem[a_seg(r->ip, SR_CS, r)];
  if(opc == 0b10001111) {
    uint8_t f = mem[a_seg(r->ip + 1, SR_CS, r)];
    if(((f >> 3) & 0b111) != 0b000) return;
    uint8_t mod = f >> 6;
    uint8_t rm = f & 0b111;
    if(mod == 0b11) {
      uint16_t *reg = regtor16(rm, r);
      *reg = fetch16(a_seg(r->sp, SR_SS, r), mem);
      r->sp += 2;
      r->ip += 2;
      return;
    } else {
      uint16_t disp = 0;
      if(mod == 0b01) disp = mem[a_seg(r->ip + 2, SR_CS, r)];
      if((mod == 0b10) || ((mod == 0b00) && rm == 0x6)) disp = fetch16(a_seg(r->ip + 3, SR_CS, r), mem);
      set16(fetch16(a_seg(r->sp, SR_SS, r), mem), ea_calc(rm, mod, disp, r, s_ovrd), mem);
      r->sp += 2;
      r->ip += 2 + mod;
      if((mod == 0b00) && rm == 0x6) r->ip += 2;
      return;
    }
  }

  if((opc >> 3) == 0b01011) {
    uint16_t *reg = regtor16(opc & 0b111, r);
    *reg = fetch16(a_seg(r->sp, SR_SS, r), mem);
    r->sp += 2;
    r->ip += 1;
    return;
  }

  if(((opc >> 5) == 0) && (opc & 0b111) == 0b111) {
    uint16_t *sreg = regtos16((opc >> 3) & 0b11, r);
    *sreg = fetch16(a_seg(r->sp, SR_SS, r), mem);
    r->sp += 2;
    r->ip += 1;
    return;
  }
}

//mmrrrggghh
void cmp(reg_t *r, uint8_t *mem) {
  uint8_t opc = mem[a_seg(r->ip, SR_CS, r)];
  if((opc >> 2) == 0b001110) {
    uint8_t f = mem[a_seg(r->ip + 1, SR_CS, r)];
    uint8_t mod = f >> 6;
    uint8_t reg = (f >> 3) & 0b111;
    uint8_t rm = f & 0b111;
    uint8_t w = opc & 1;
    uint8_t d = (opc >> 1) & 1;
    *(uint16_t *)r->flg &= ~(SF | ZF | CF | PF | OF);
    if(mod == 0b11) {
      if(w) {
        uint16_t a = *regtor16(rm, r);
        uint16_t b = *regtor16(reg, r);
        int16_t sum = 0;

        if(!d) sum = a - (int16_t)b;
        else sum = b - (int16_t)a;

        if(sum < 0) *(uint16_t *)r->flg |= SF;
        if(sum == 0) *(uint16_t *)r->flg |= ZF;
        if((sum < 0) && ((int16_t)a >= 0) && ((int16_t)b >= 0)) *(uint16_t *)r->flg |= CF;
        if(parchk16(sum)) *(uint16_t *)r->flg |= PF;
        if(((int16_t)b > 0 && (int16_t)a < INT16_MIN + (int16_t)b) || ((int16_t)b < 0 && (int16_t)a > INT16_MAX + (int16_t)b)) *(uint16_t *)r->flg |= OF;

        r->ip += 2;
        return;
      } else {
        uint8_t a = *regtor8(rm, r);
        uint8_t b = *regtor8(reg, r);
        int8_t sum = 0;

        if(!d) sum = a - (int8_t)b;
        else sum = b - (int8_t)a;

        if(sum < 0) *(uint16_t *)r->flg |= SF;
        if(sum == 0) *(uint16_t *)r->flg |= ZF;
        if((sum < 0) && ((int8_t)a >= 0) && ((int8_t)b >= 0)) *(uint16_t *)r->flg |= CF;
        if(parchk8(sum)) *(uint16_t *)r->flg |= PF;
        if(((int8_t)b > 0 && (int8_t)a < INT8_MIN + (int8_t)b) || ((int8_t)b < 0 && (int8_t)a > INT8_MAX + (int8_t)b)) *(uint16_t *)r->flg |= OF;

        r->ip += 2;
        return;
      }
    } else {
      uint16_t disp = 0;
      if(mod == 0b01) disp = mem[a_seg(r->ip + 2, SR_CS, r)];
      if((mod == 2) || ((mod == 0) && (rm == 0x06))) disp = fetch16(a_seg(r->ip + 2, SR_CS, r), mem);
      if(w) {
        uint16_t a = fetch16(ea_calc(rm, mod, disp, r, s_ovrd), mem);
        uint16_t b = *regtor16(reg, r);
        int16_t sum = 0;

        if(!d) sum = a - (int16_t)b;
        else sum = b - (int16_t)a;

        if(sum < 0) *(uint16_t *)r->flg |= SF;
        if(sum == 0) *(uint16_t *)r->flg |= ZF;
        if((sum < 0) && ((int16_t)a >= 0) && ((int16_t)b >= 0)) *(uint16_t *)r->flg |= CF;
        if(parchk16(sum)) *(uint16_t *)r->flg |= PF;
        if(((int16_t)b > 0 && (int16_t)a < INT16_MIN + (int16_t)b) || ((int16_t)b < 0 && (int16_t)a > INT16_MAX + (int16_t)b)) *(uint16_t *)r->flg |= OF;

        r->ip += 2 + mod;
        if((mod == 0) && (rm == 0x6)) r->ip += 2;
        return;
      } else {
        uint8_t a = mem[ea_calc(rm, mod, disp, r, s_ovrd)];
        uint8_t b = *regtor8(reg, r);
        int8_t sum = 0;

        if(!d) sum = a - (int8_t)b;
        else sum = b - (int8_t)a;

        if(sum < 0) *(uint16_t *)r->flg |= SF;
        if(sum == 0) *(uint16_t *)r->flg |= ZF;
        if((sum < 0) && ((int8_t)a >= 0) && ((int8_t)b >= 0)) *(uint16_t *)r->flg |= CF;
        if(parchk8(sum)) *(uint16_t *)r->flg |= PF;
        if(((int8_t)b > 0 && (int8_t)a < INT8_MIN + (int8_t)b) || ((int8_t)b < 0 && (int8_t)a > INT8_MAX + (int8_t)b)) *(uint16_t *)r->flg |= OF;

        r->ip += 2 + mod;
        if((mod == 0) && (rm == 0x6)) r->ip += 2;
        return;
      }
    }
  }

  if(opc >> 2 == 0b100000) {
    uint8_t f = mem[a_seg(r->ip + 1, SR_CS, r)];
    if(((f >> 3) & 0b111) != 0b111) return;
    uint8_t rm = f & 0b111;
    uint8_t mod = f >> 6;
    uint8_t sw = opc & 0b11;
    *(uint16_t *)r->flg &= ~(SF | ZF | CF | PF | OF);
    if(mod == 0b11) {
      if(sw & 1) {
        int16_t a = *(int16_t *)regtor16(rm, r);
        int16_t b = 0;
        if(sw == 0b01) b = fetch16(a_seg(r->ip + 2, SR_CS, r), mem);
        if(sw == 0b11) b = (int16_t)(int8_t)mem[a_seg(r->ip + 2, SR_CS, r)];

        int16_t sum = a - b;

        if(sum < 0) *(uint16_t *)r->flg |= SF;
        if(sum == 0) *(uint16_t *)r->flg |= ZF;
        if((sum < 0) && ((int16_t)a >= 0) && ((int16_t)b >= 0)) *(uint16_t *)r->flg |= CF;
        if(parchk16(sum)) *(uint16_t *)r->flg |= PF;
        if(((int16_t)b > 0 && (int16_t)a < INT16_MIN + (int16_t)b) || ((int16_t)b < 0 && (int16_t)a > INT16_MAX + (int16_t)b)) *(uint16_t *)r->flg |= OF;

        r->ip += 2;
        if(sw == 0b01) r->ip += 2;
        if(sw == 0b11) r->ip += 1;
        return;
      } else {
        int8_t a = *(int8_t *)regtor8(rm, r);
        int8_t b = 0;
        b = (int8_t)mem[a_seg(r->ip + 2, SR_CS, r)];

        int8_t sum = a - b;

        if(sum < 0) *(uint16_t *)r->flg |= SF;
        if(sum == 0) *(uint16_t *)r->flg |= ZF;
        if((sum < 0) && ((int16_t)a >= 0) && ((int16_t)b >= 0)) *(uint16_t *)r->flg |= CF;
        if(parchk8(sum)) *(uint16_t *)r->flg |= PF;
        if(((int8_t)b > 0 && (int8_t)a < INT8_MIN + (int8_t)b) || ((int8_t)b < 0 && (int8_t)a > INT16_MAX + (int8_t)b)) *(uint16_t *)r->flg |= OF;

        r->ip += 3;
        return;
      }
    } else {
      uint16_t disp = 0;
      if(mod == 0b01) disp = mem[a_seg(r->ip + 2, SR_CS, r)];
      if((mod == 2) || ((mod == 0) && (rm == 0x06))) disp = fetch16(a_seg(r->ip + 2, SR_CS, r), mem);

      if(sw & 1) {
        int16_t a = fetch16(ea_calc(rm, mod, disp, r, s_ovrd), mem);
        int16_t b = 0;

        r->ip += 2 + mod;
        if((mod == 0) && (rm == 0x6)) r->ip += 2;
        if(sw == 0b01) b = fetch16(a_seg(r->ip, SR_CS, r), mem);
        if(sw == 0b11) b = (int16_t)(int8_t)mem[a_seg(r->ip, SR_CS, r)];
        int16_t sum = a - b;

        if(sum < 0) *(uint16_t *)r->flg |= SF;
        if(sum == 0) *(uint16_t *)r->flg |= ZF;
        if((sum < 0) && ((int16_t)a >= 0) && ((int16_t)b >= 0)) *(uint16_t *)r->flg |= CF;
        if(parchk16(sum)) *(uint16_t *)r->flg |= PF;
        if(((int16_t)b > 0 && (int16_t)a < INT16_MIN + (int16_t)b) || ((int16_t)b < 0 && (int16_t)a > INT16_MAX + (int16_t)b)) *(uint16_t *)r->flg |= OF;

        if(sw == 0b01) r->ip += 2;
        if(sw == 0b11) r->ip += 1;
        return;
      } else {
        int8_t a = (int8_t)mem[ea_calc(rm, mod, disp, r, s_ovrd)];
        int8_t b = 0;

        r->ip += 2 + mod;
        if((mod == 0) && (rm == 0x6)) r->ip += 2;
        b = (int8_t)mem[a_seg(r->ip, SR_CS, r)];
        int8_t sum = a - b;

        if(sum < 0) *(uint16_t *)r->flg |= SF;
        if(sum == 0) *(uint16_t *)r->flg |= ZF;
        if((sum < 0) && ((int16_t)a >= 0) && ((int16_t)b >= 0)) *(uint16_t *)r->flg |= CF;
        if(parchk8(sum)) *(uint16_t *)r->flg |= PF;
        if(((int8_t)b > 0 && (int8_t)a < INT8_MIN + (int8_t)b) || ((int8_t)b < 0 && (int8_t)a > INT8_MAX + (int8_t)b)) *(uint16_t *)r->flg |= OF;

        r->ip += 1;
        return;
      }
    }
  }

  if(opc >> 1 == 0b0011110) {
    uint8_t w = opc & 1;
    *(uint16_t *)r->flg &= ~(SF | ZF | CF | PF | OF);
    if(w) {
      int16_t a = *(uint16_t *)r->ax;
      int16_t b = fetch16(a_seg(r->ip + 1, SR_CS, r), mem);
      int16_t sum = a - b;

      if(sum < 0) *(uint16_t *)r->flg |= SF;
      if(sum == 0) *(uint16_t *)r->flg |= ZF;
      if((sum < 0) && ((int16_t)a >= 0) && ((int16_t)b >= 0)) *(uint16_t *)r->flg |= CF;
      if(parchk16(sum)) *(uint16_t *)r->flg |= PF;
      if(((int16_t)b > 0 && (int16_t)a < INT16_MIN + (int16_t)b) || ((int16_t)b < 0 && (int16_t)a > INT16_MAX + (int16_t)b)) *(uint16_t *)r->flg |= OF;

      r->ip += 3;
      return;
    } else {
      int8_t a = r->ax[0];
      int8_t b = mem[a_seg(r->ip + 1, SR_CS, r)];
      int8_t sum = a - b;

      if(sum < 0) *(uint16_t *)r->flg |= SF;
      if(sum == 0) *(uint16_t *)r->flg |= ZF;
      if((sum < 0) && ((int8_t)a >= 0) && ((int8_t)b >= 0)) *(uint16_t *)r->flg |= CF;
      if(parchk8(sum)) *(uint16_t *)r->flg |= PF;
      if(((int8_t)b > 0 && (int8_t)a < INT8_MIN + (int8_t)b) || ((int8_t)b < 0 && (int8_t)a > INT8_MAX + (int8_t)b)) *(uint16_t *)r->flg |= OF;

      r->ip += 2;
      return;
    }
  }
}

void intr(reg_t *r, uint8_t *mem) {
  uint8_t opc = mem[a_seg(r->ip, SR_CS, r)];
  if(opc == 0b11001101) {
    uint8_t typ = mem[a_seg(r->ip + 1, SR_CS, r)];
    intrt_h(typ, r, mem, r->ip + 2);
    return;
  }
  if(opc == 0b11001100) {
    intrt_h(3, r, mem, r->ip + 1);
    return;
  }
  if(opc == 0b11001110) {
    if(*(uint16_t *)r->flg & OF) intrt_h(4, r, mem, r->ip + 1);
    else r->ip += 1;
    return;
  }
  if(opc == 0b11001111) { // IRET
    r->ip = fetch16(a_seg(r->sp, SR_SS, r), mem);
    r->sp += 2;
    r->cs = fetch16(a_seg(r->sp, SR_SS, r), mem);
    r->sp += 2;
    *(uint16_t *)r->flg = fetch16(a_seg(r->sp, SR_SS, r), mem);
    r->sp += 2;
    return;
  }
}

void jmp(reg_t *r, uint8_t *mem) {
  uint8_t opc = mem[a_seg(r->ip, SR_CS, r)];
  if(opc == 0b11101001) {
    int16_t disp = fetch16(a_seg(r->ip + 1, SR_CS, r), mem);
    r->ip += disp + 3;
    return;
  }

  if(opc == 0b11101011) {
    int8_t disp = mem[a_seg(r->ip + 1, SR_CS, r)];
    r->ip += (int16_t)disp + 2;
    return;
  }

  if(opc == 0b11111111) {
    uint8_t f = mem[a_seg(r->ip + 1, SR_CS, r)];
    if(((f >> 3) & 0b111) != 0b100) return;
    uint8_t mod = f >> 6;
    uint8_t rm = f & 0b111;
    if(mod == 0b11) {
      int16_t disp = *(int16_t *)regtor16(rm, r);
      r->ip += disp + 2;
      return;
    } else {
      uint16_t disp = 0;
      if(mod == 0b01) disp = mem[a_seg(r->ip + 2, SR_CS, r)];
      if((mod == 2) || ((mod == 0) && (rm == 0x06))) disp = fetch16(a_seg(r->ip + 2, SR_CS, r), mem);
      int16_t a = fetch16(ea_calc(rm, mod, disp, r, s_ovrd), mem);
      r->ip += a + 2 + mod;
      if((mod == 0) && (rm == 0x06)) r->ip += 2;
      return;
    }
  }

  if(opc == 0b11101010) {
    uint16_t off = fetch16(a_seg(r->ip + 1, SR_CS, r), mem);
    uint16_t seg = fetch16(a_seg(r->ip + 3, SR_CS, r), mem);
    r->cs = seg;
    r->ip = off;
    return;
  }
}

void call(reg_t *r, uint8_t *mem) {
  uint8_t opc = mem[a_seg(r->ip, SR_CS, r)];
  if(opc == 0b11101000) {
    uint16_t disp = fetch16(a_seg(r->ip + 1, SR_CS, r), mem);
    r->sp -= 2;
    set16(r->ip + 3, a_seg(r->sp, SR_SS, r), mem);
    r->ip += disp + 3;
    return;
  }

  if(opc == 0b11111111) {
    uint8_t f = mem[a_seg(r->ip + 1, SR_CS, r)];
    if(((f >> 3) & 0b111) != 0b010) return;
    uint8_t rm = f & 0b111;
    uint8_t mod = f >> 6;
    if(mod == 0b11) {
      int16_t disp = *regtor16(rm, r);
      r->sp -= 2;
      set16(r->ip + 3, a_seg(r->sp, SR_SS, r), mem);
      r->ip += disp + 2;
    } else {
      uint16_t disp = 0;
      if(mod == 0b01) disp = mem[a_seg(r->ip + 2, SR_CS, r)];
      if((mod == 2) || ((mod == 0) && (rm == 0x06))) disp = fetch16(a_seg(r->ip + 2, SR_CS, r), mem);
      int16_t a = fetch16(ea_calc(rm, mod, disp, r, s_ovrd), mem);

      uint16_t n = r->ip + 2 + mod;
      if((mod == 0) && (rm == 0x06)) n += 2;
      r->sp -= 2;
      set16(n, a_seg(r->sp, SR_SS, r), mem);

      r->ip += a + 2 + mod;
      if((mod == 0) && (rm == 0x06)) r->ip += 2;
      return;
    }
  }

  if(opc == 0b10011010) {
    uint16_t off = fetch16(a_seg(r->ip + 1, SR_CS, r), mem);
    uint16_t seg = fetch16(a_seg(r->ip + 3, SR_CS, r), mem);
    r->sp -= 2;
    set16(r->cs, a_seg(r->sp, SR_SS, r), mem);
    r->sp -= 2;
    set16(r->ip + 5, a_seg(r->sp, SR_SS, r), mem);
    r->cs = seg;
    r->ip = off;
    return;
  }
}

void ret(reg_t *r, uint8_t *mem) {
  uint8_t opc = mem[a_seg(r->ip, SR_CS, r)];
  if(opc == 0b11000011) {
    r->ip = fetch16(a_seg(r->sp, SR_SS, r), mem);
    r->sp += 2;
    return;
  }

  if(opc == 0b11000010) {
    uint16_t imm = fetch16(a_seg(r->ip + 1, SR_CS, r), mem);
    r->ip = fetch16(a_seg(r->sp, SR_SS, r), mem);
    r->sp += 2 + imm;
    return;
  }

  if(opc == 0b11001011) {
    r->ip = fetch16(a_seg(r->sp, SR_SS, r), mem);
    r->sp += 2;
    r->cs = fetch16(a_seg(r->sp, SR_SS, r), mem);
    r->sp += 2;
    return;
  }

  if(opc == 0b1100101) {
    uint16_t imm = fetch16(a_seg(r->ip + 1, SR_CS, r), mem);
    r->ip = fetch16(a_seg(r->sp, SR_SS, r), mem);
    r->sp += 2;
    r->cs = fetch16(a_seg(r->sp, SR_SS, r), mem);
    r->sp += 2 + imm;
    return;
  }
}

void jcc(reg_t *r, uint8_t *mem) {
  uint8_t opc = mem[a_seg(r->ip, SR_CS, r)];
  int8_t disp = mem[a_seg(r->ip + 1, SR_CS, r)];
  switch(opc) {
  case 0b01110100:
    if(*(uint16_t *)r->flg & ZF) r->ip += disp;
    break;
  case 0b01111100:
    if(((*(uint16_t *)r->flg & SF) ? 1 : 0) != ((*(uint16_t *)r->flg & OF) ? 1 : 0)) r->ip += disp;
    break;
  case 0b01111110:
    if(((*(uint16_t *)r->flg & SF) ? 1 : 0) != ((*(uint16_t *)r->flg & OF) ? 1 : 0) || *(uint16_t *)r->flg & ZF) r->ip += disp;
    break;
  case 0b01110010:
    if(*(uint16_t *)r->flg & CF) r->ip += disp;
    break;
  case 0b01110110:
    if(*(uint16_t *)r->flg & CF || *(uint16_t *)r->flg & ZF) r->ip += disp;
    break;
  case 0b01111010:
    if(*(uint16_t *)r->flg & PF) r->ip += disp;
    break;
  case 0b01110000:
    if(*(uint16_t *)r->flg & OF) r->ip += disp;
    break;
  case 0b01111000:
    if(*(uint16_t *)r->flg & SF) r->ip += disp;
    break;
  case 0b01110101:
    if(!(*(uint16_t *)r->flg & ZF)) r->ip += disp;
    break;
  case 0b01111101:
    if(((*(uint16_t *)r->flg & SF) ? 1 : 0) == ((*(uint16_t *)r->flg & OF) ? 1 : 0)) r->ip += disp;
    break;
  case 0b01111111:
    if(((*(uint16_t *)r->flg & CF) ? 1 : 0) == ((*(uint16_t *)r->flg & OF) ? 1 : 0) || (*(uint16_t *)r->flg & ZF) == 0) r->ip += disp;
    // what an interesting line
    break;
  case 0b01110011:
    if((*(uint16_t *)r->flg & CF) + (*(uint16_t *)r->flg & ZF) == 0) r->ip += disp;
    break;
  case 0b01111011:

    if((*(uint16_t *)r->flg & PF) == 0) r->ip += disp;
    break;
  case 0b01110001:
    if((*(uint16_t *)r->flg & OF) == 0) r->ip += disp;
    break;
  case 0b01111001:
    if((*(uint16_t *)r->flg & SF) == 0) r->ip += disp;
    break;
  case 0b11100011:
    if(*(uint16_t *)r->cx == 0) r->ip += disp;
    break;
  default:
    return;
  }
  r->ip += 2;
  return;
}

void loop(reg_t *r, uint8_t *mem) {
  uint8_t opc = mem[a_seg(r->ip, SR_CS, r)];
  int8_t disp = mem[a_seg(r->ip + 1, SR_CS, r)];
  if(opc == 0b11100010) {
    if(--(*(uint16_t *)r->cx) != 0) r->ip += disp + 2;
    else r->ip += 2;
    return;
  }
  if(opc == 0b11100001) {
    if(--(*(uint16_t *)r->cx) != 0 || (*(uint16_t *)r->flg & ZF)) r->ip += disp + 2;
    else r->ip += 2;
    return;
  }
  if(opc == 0b11100000) {
    if(--(*(uint16_t *)r->cx) != 0 || (*(uint16_t *)r->flg & ZF) == 0) r->ip += disp + 2;
    else r->ip += 2;
    return;
  }
}

void inout(reg_t *r, uint8_t *mem) {
  uint8_t opc = mem[a_seg(r->ip, SR_CS, r)];
  if((opc >> 1) == 0b1110011) {
    uint8_t port = mem[a_seg(r->ip + 1, SR_CS, r)];
    uint8_t w = opc & 1;
    if(w) {
      handleout((uint16_t)port, *(uint16_t *)r->ax, 1);
      r->ip += 2;
      return;
    } else {
      handleout((uint16_t)port, (uint16_t)r->ax[0], 0);
      r->ip += 2;
      return;
    }
  }

  if((opc >> 1) == 0b1110111) {
    uint8_t w = opc & 1;
    if(w) {
      handleout(*(uint16_t *)r->dx, *(uint16_t *)r->ax, 1);
      r->ip += 1;
      return;
    } else {
      handleout(*(uint16_t *)r->dx, r->ax[0], 0);
      r->ip += 1;
      return;
    }
  }

  if((opc >> 1) == 0b1110010) {
    uint8_t port = mem[a_seg(r->ip + 1, SR_CS, r)];
    uint8_t w = opc & 1;
    if(w) {
      *(uint16_t *)r->ax = handlein(port, 1);
      r->ip += 2;
      return;
    } else {
      r->ax[0] = handlein(port, 0);
      r->ip += 2;
      return;
    }
  }

  if((opc >> 1) == 0b1110110) {
    uint8_t w = opc & 1;
    if(w) {
      *(uint16_t *)r->ax = handlein(*(uint16_t *)r->dx, 1);
      r->ip += 1;
      return;
    } else {
      r->ax[0] = handlein(*(uint16_t *)r->dx, 0);
      r->ip += 1;
      return;
    }
  }
}

void pctrl(reg_t *r, uint8_t *mem) {
#define FLG *(uint16_t*)r->flg
  uint8_t opc = mem[a_seg(r->ip, SR_CS, r)];
  switch(opc) {
  case 0b11111000:
    FLG &= ~CF;
    break;
  case 0b11110101:
    if(FLG & CF) FLG &= ~CF;
    else FLG |= CF;
    break;
  case 0b11111001:
    FLG |= CF;
    break;
  case 0b11111100:
    FLG &= ~DF;
    break;
  case 0b11111101:
    FLG |= DF;
    break;
  case 0b11111010:
    FLG &= ~IF;
    break;
  case 0b11111011:
    FLG |= IF;
    break;
  case 0b11110100:
    printf("CPU halted...\n");
    return;
    break;
  case 0b10011011:
    break;
  case 0b11110000:
    break;
  default:
    return;
  }

  r->ip += 1;
  return;
}

void xchg(reg_t *r, uint8_t *mem) {
  uint8_t opc = mem[a_seg(r->ip, SR_CS, r)];

  if((opc >> 1) == 0b1000011) {
    uint8_t f = mem[a_seg(r->ip + 1, SR_CS, r)];
    uint8_t w = opc & 1;
    uint8_t rm = f & 0b111;
    uint8_t reg = (f >> 3) & 0b111;
    uint8_t mod = f >> 6;
    if(mod == 0b11) {
      if(w) {
        uint16_t *a = regtor16(rm, r);
        uint16_t *b = regtor16(reg, r);
        uint16_t tmp = *a;
        *a = *b;
        *b = tmp;
        r->ip += 2;
        return;
      } else {
        uint8_t *a = regtor8(rm, r);
        uint8_t *b = regtor8(reg, r);
        uint8_t tmp = *a;
        *a = *b;
        *b = tmp;
        r->ip += 2;
        return;
      }
    } else {
      uint16_t disp = 0;
      if(mod == 0b01) disp = mem[a_seg(r->ip + 2, SR_CS, r)];
      if((mod == 2) || ((mod == 0) && (rm == 0x06))) disp = fetch16(a_seg(r->ip + 2, SR_CS, r), mem);
      if(w) {
        uint16_t *a = (uint16_t *)&mem[ea_calc(rm, mod, disp, r, s_ovrd)];
        uint16_t *b = regtor16(reg, r);
        uint16_t tmp = *a;
        *a = *b;
        *b = tmp;
      } else {
        uint8_t *a = (uint8_t *)&mem[ea_calc(rm, mod, disp, r, s_ovrd)];
        uint8_t *b = regtor8(reg, r);
        uint8_t tmp = *a;
        *a = *b;
        *b = tmp;
      }
      r->ip += 2 + mod;
      if((mod == 0) && (rm == 0x06)) r->ip += 2;
      return;
    }
  }

  if((opc >> 3) == 0b10010) {
    uint8_t reg = opc & 0b111;
    uint16_t *b = regtor16(reg, r);
    uint16_t tmp = *b;
    *b = *(uint16_t *)r->ax;
    *(uint16_t *)r->ax = tmp;
    r->ip += 1;
    return;
  }
}

void xlat(reg_t *r, uint8_t *mem) {
  uint8_t opc = mem[a_seg(r->ip, SR_CS, r)];
  if(opc == 0b11010111) {
    r->ax[0] = mem[a_seg(*(uint16_t *)r->bx + r->ax[0], SR_DS, r)];
    r->ip += 1;
    return;
  }
}

void lea(reg_t *r, uint8_t *mem) {
  uint8_t opc = CURBYTE;
  if(opc == 0b10001101) {
    uint8_t f = NXTBYTE;
    uint8_t rm = f & 0b111;
    uint8_t reg = (f >> 3) & 0b111;
    uint8_t mod = f >> 6;
    if(mod == 0b11) {
      uint16_t *d = regtor16(reg, r);
      uint16_t ea = *regtor16(rm, r);
      *d = ea;
      r->ip += 2;
      return;
    } else {
      uint16_t disp = 0;
      if(mod == 0b01) disp = mem[a_seg(r->ip + 2, SR_CS, r)];
      if((mod == 2) || ((mod == 0) && (rm == 0x06))) disp = fetch16(a_seg(r->ip + 2, SR_CS, r), mem);

      uint16_t ea = ea_calc(rm, mod, disp, r, SR_NONE);
      uint16_t *d = regtor16(reg, r);
      *d = ea;

      r->ip += 2 + mod;
      if((mod == 0) && (rm == 0x06)) r->ip += 2;
      return;
    }
  }
}

void lds(reg_t *r, uint8_t *mem) {
  uint8_t opc = CURBYTE;
  if(opc == 0b11000101) {
    uint8_t f = NXTBYTE;
    uint8_t rm = f & 0b111;
    uint8_t reg = (f >> 3) & 0b111;
    uint8_t mod = f >> 6;
    if(mod == 0b11) {
      uint16_t *d = regtor16(reg, r);
      uint16_t ea = *regtor16(rm, r);
      *d = fetch16(a_seg(ea, SR_DS, r), mem);
      r->ds = fetch16(a_seg(ea + 2, SR_DS, r), mem);
      r->ip += 2;
      return;
    } else {
      uint16_t disp = 0;
      if(mod == 0b01) disp = mem[a_seg(r->ip + 2, SR_CS, r)];
      if((mod == 2) || ((mod == 0) && (rm == 0x06))) disp = fetch16(a_seg(r->ip + 2, SR_CS, r), mem);
      uint32_t ea = ea_calc(rm, mod, disp, r, s_ovrd);
      uint16_t *d = regtor16(reg, r);

      *d = fetch16(ea, mem);
      r->ds = fetch16(ea + 2, mem);

      r->ip += 2 + mod;
      if((mod == 0) && (rm == 0x06)) r->ip += 2;
      return;
    }
  }
}

void les(reg_t *r, uint8_t *mem) {
  uint8_t opc = CURBYTE;
  if(opc == 0b11000100) {
    uint8_t f = NXTBYTE;
    uint8_t rm = f & 0b111;
    uint8_t reg = (f >> 3) & 0b111;
    uint8_t mod = f >> 6;
    if(mod == 0b11) {
      uint16_t *d = regtor16(reg, r);
      uint16_t ea = *regtor16(rm, r);
      *d = fetch16(a_seg(ea, SR_DS, r), mem);
      r->es = fetch16(a_seg(ea + 2, SR_DS, r), mem);
      r->ip += 2;
      return;
    } else {
      uint16_t disp = 0;
      if(mod == 0b01) disp = mem[a_seg(r->ip + 2, SR_CS, r)];
      if((mod == 2) || ((mod == 0) && (rm == 0x06))) disp = fetch16(a_seg(r->ip + 2, SR_CS, r), mem);
      uint32_t ea = ea_calc(rm, mod, disp, r, s_ovrd);
      uint16_t *d = regtor16(reg, r);

      *d = fetch16(ea, mem);
      r->es = fetch16(ea + 2, mem);

      r->ip += 2 + mod;
      if((mod == 0) && (rm == 0x06)) r->ip += 2;
      return;
    }
  }
}

void lsahf(reg_t *r, uint8_t *mem) {
  uint8_t opc = CURBYTE;
  if(opc == 0b10011111) {
    r->ax[1] = r->flg[0];
    r->ip += 1;
    return;
  }

  if(opc == 0b10011110) {
    r->flg[0] = r->ax[1];
    r->ip += 1;
    return;
  }

  if(opc == 0b10011100) {
    r->sp -= 2;
    set16(*(uint16_t *)r->flg, a_seg(r->sp, SR_SS, r), mem);
    r->ip += 1;
    return;
  }

  if(opc == 0b10011101) {
    *(uint16_t *)r->flg = fetch16(a_seg(r->sp, SR_CS, r), mem);
    r->sp += 2;
    r->ip += 1;
    return;
  }
}


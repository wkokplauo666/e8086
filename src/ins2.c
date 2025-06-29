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
#define FLG *(uint16_t*)r->flg

void add(reg_t *r, uint8_t *mem) {
  uint8_t opc = CURBYTE;
  if((opc >> 2) == 0) {
    uint8_t f = NXTBYTE;
    uint8_t mod = f >> 6;
    uint8_t reg = (f >> 3) & 0b111;
    uint8_t rm = f & 0b111;
    uint8_t d = (opc >> 1) & 1;
    uint8_t w = opc & 1;
    FLG &= ~(SF | ZF | CF | OF | PF);
    if(mod == 0b11) {
      if(w) {
        int16_t *a = (int16_t *)regtor16(rm, r);
        int16_t *b = (int16_t *)regtor16(reg, r);
        int16_t sum = *a + *b;
        if(sum < 0) FLG |= SF;
        if(sum == 0) FLG |= ZF;
        if(((uint32_t)*a + (uint32_t)*b) > UINT16_MAX) FLG |= CF;
        if((*a > INT16_MAX - *b) || (*a < INT16_MIN - *b)) FLG |= OF;
        if(parchk16(sum)) FLG |= PF;
        if(!d) *a = sum;
        else *b = sum;
        r->ip += 2;
        return;
      } else {
        int8_t *a = (int8_t *)regtor8(rm, r);
        int8_t *b = (int8_t *)regtor8(reg, r);
        int8_t sum = *a + *b;
        if(sum < 0) FLG |= SF;
        if(sum == 0) FLG |= ZF;
        if(((uint16_t)*a + (uint16_t)*b) > UINT8_MAX) FLG |= CF;
        if((*a > INT8_MAX - *b) || (*a < INT8_MIN - *b)) FLG |= OF;
        if(parchk8(sum)) FLG |= PF;
        if(!d) *a = sum;
        else *b = sum;
        r->ip += 2;
        return;
      }
    } else {
      uint16_t disp = 0;
      if(mod == 0b01) disp = mem[a_seg(r->ip + 2, SR_CS, r)];
      if((mod == 2) || ((mod == 0) && (rm == 0x06))) disp = fetch16(a_seg(r->ip + 2, SR_CS, r), mem);
      if(w) {
        int16_t *a = (int16_t *)&mem[ea_calc(rm, mod, disp, r, s_ovrd)];
        int16_t *b = (int16_t *)regtor16(reg, r);
        int16_t sum = *a + *b;
        if(sum < 0) FLG |= SF;
        if(sum == 0) FLG |= ZF;
        if(((uint32_t)*a + (uint32_t)*b) > UINT16_MAX) FLG |= CF;
        if((*a > INT16_MAX - *b) || (*a < INT16_MIN - *b)) FLG |= OF;
        if(parchk16(sum)) FLG |= PF;

        if(!d) *a = sum;
        else *b = sum;
      } else {
        int8_t *a = (int8_t *)&mem[ea_calc(rm, mod, disp, r, s_ovrd)];
        int8_t *b = (int8_t *)regtor8(reg, r);
        int8_t sum = *a + *b;
        if(sum < 0) FLG |= SF;
        if(sum == 0) FLG |= ZF;
        if(((uint16_t)*a + (uint16_t)*b) > UINT8_MAX) FLG |= CF;
        if((*a > INT8_MAX - *b) || (*a < INT8_MIN - *b)) FLG |= OF;
        if(parchk8(sum)) FLG |= PF;
        if(!d) *a = sum;
        else *b = sum;
      }

      r->ip += 2 + mod;
      if((mod == 0) && (rm == 0x06)) r->ip += 2;
      return;
    }
  }

  if((opc) >> 2 == 0b100000) {
    uint8_t f = mem[a_seg(r->ip + 1, SR_CS, r)];
    if(((f >> 3) & 0b111) != 0b000) return;
    uint8_t rm = f & 0b111;
    uint8_t mod = f >> 6;
    uint8_t sw = opc & 0b11;
    *(uint16_t *)r->flg &= ~(SF | ZF | CF | PF | OF);
    if(mod == 0b11) {
      if(sw & 1) {
        int16_t *a = (int16_t *)regtor16(rm, r);
        int16_t b = 0;
        if(sw == 0b01) b = (int16_t)mem[a_seg(r->ip + 2, SR_CS, r)];
        if(sw == 0b11) b = (int16_t)((int8_t *)mem)[a_seg(r->ip + 2, SR_CS, r)];

        int16_t sum = *a + b;

        if(sum < 0) FLG |= SF;
        if(sum == 0) FLG |= ZF;
        if(((uint32_t)*a + (uint32_t)b) > UINT16_MAX) FLG |= CF;
        if((*a > INT16_MAX - b) || (*a < INT16_MIN - b)) FLG |= OF;
        if(parchk16(sum)) FLG |= PF;

        *a = sum;

        r->ip += 2;
        if(sw == 0b01) r->ip += 2;
        if(sw == 0b11) r->ip += 1;
        return;
      } else {
        int8_t *a = (int8_t *)regtor8(rm, r);
        int8_t *b = (int8_t *)&mem[a_seg(r->ip + 2, SR_CS, r)];
        int8_t sum = *a + *b;

        if(sum < 0) FLG |= SF;
        if(sum == 0) FLG |= ZF;
        if(((uint32_t)*a + (uint32_t)*b) > UINT8_MAX) FLG |= CF;
        if((*a > INT8_MAX - *b) || (*a < INT8_MIN - *b)) FLG |= OF;
        if(parchk8(sum)) FLG |= PF;

        *a = sum;

        r->ip += 3;
        return;
      }
    } else {
      uint16_t disp = 0;
      if(mod == 0b01) disp = mem[a_seg(r->ip + 2, SR_CS, r)];
      if((mod == 2) || ((mod == 0) && (rm == 0x06))) disp = fetch16(a_seg(r->ip + 2, SR_CS, r), mem);
      if(sw & 1) {
        int16_t *a = (int16_t *)&mem[ea_calc(rm, mod, disp, r, s_ovrd)];
        int16_t b = 0;

        r->ip += 2 + mod;
        if((mod == 0) && (rm == 0x6)) r->ip += 2;
        if(sw == 0b01) b = *(int16_t *)&mem[a_seg(r->ip, SR_CS, r)];
        if(sw == 0b11) b = (int16_t)((int8_t *)mem)[a_seg(r->ip, SR_CS, r)];
        int16_t sum = *a + b;

        if(sum < 0) FLG |= SF;
        if(sum == 0) FLG |= ZF;
        if(((uint32_t)*a + (uint32_t)b) > UINT16_MAX) FLG |= CF;
        if((*a > INT16_MAX - b) || (*a < INT16_MIN - b)) FLG |= OF;
        if(parchk16(sum)) FLG |= PF;

        *a = sum;

        if(sw == 0b01) r->ip += 2;
        if(sw == 0b11) r->ip += 1;
        return;
      } else {
        int8_t *a = (int8_t *)&mem[ea_calc(rm, mod, disp, r, s_ovrd)];

        r->ip += 2 + mod;
        if((mod == 0) && (rm == 0x6)) r->ip += 2;
        int8_t *b = (int8_t *)&mem[a_seg(r->ip, SR_CS, r)];
        int8_t sum = *a + *b;

        if(sum < 0) FLG |= SF;
        if(sum == 0) FLG |= ZF;
        if(((uint32_t)*a + (uint32_t)*b) > UINT8_MAX) FLG |= CF;
        if((*a > INT8_MAX - *b) || (*a < INT8_MIN - *b)) FLG |= OF;
        if(parchk8(sum)) FLG |= PF;

        *a = sum;
        return;
      }
    }
  }

  if((opc >> 1) == 0b0000010) {
    uint8_t w = opc & 1;
    if(w) {
      int16_t *a = (int16_t *)r->ax;
      int16_t *b = (int16_t *)&mem[a_seg(r->ip + 1, SR_CS, r)];

      int16_t sum = *a + *b;
      if(sum < 0) FLG |= SF;
      if(sum == 0) FLG |= ZF;
      if(((uint32_t)*a + (uint32_t)*b) > UINT16_MAX) FLG |= CF;
      if((*a > INT16_MAX - *b) || (*a < INT16_MIN - *b)) FLG |= OF;
      if(parchk16(sum)) FLG |= PF;

      *a = sum;
      r->ip += 3;
      return;
    } else {
      int8_t *a = (int8_t *)&r->ax[0];
      int8_t *b = (int8_t *)&mem[a_seg(r->ip + 1, SR_CS, r)];

      int8_t sum = *a + *b;
      if(sum < 0) FLG |= SF;
      if(sum == 0) FLG |= ZF;
      if(((uint32_t)*a + (uint32_t)*b) > UINT8_MAX) FLG |= CF;
      if((*a > INT8_MAX - *b) || (*a < INT8_MIN - *b)) FLG |= OF;
      if(parchk8(sum)) FLG |= PF;

      *a = sum;
      r->ip += 2;
      return;
    }
  }
}

void adc(reg_t *r, uint8_t *mem) {
  uint8_t opc = CURBYTE;
  uint8_t cstate = (FLG & CF) ? 1 : 0;
  if((opc >> 2) == 0b000100) {
    uint8_t f = NXTBYTE;
    uint8_t mod = f >> 6;
    uint8_t reg = (f >> 3) & 0b111;
    uint8_t rm = f & 0b111;
    uint8_t d = (opc >> 1) & 1;
    uint8_t w = opc & 1;
    FLG &= ~(SF | ZF | CF | OF | PF);
    if(mod == 0b11) {
      if(w) {
        int16_t *a = (int16_t *)regtor16(rm, r);
        int16_t *b = (int16_t *)regtor16(reg, r);
        if(!d) *a += cstate;
        else *b += cstate;
        int16_t sum = *a + *b;
        if(sum < 0) FLG |= SF;
        if(sum == 0) FLG |= ZF;
        if(((uint32_t)*a + (uint32_t)*b) > UINT16_MAX) FLG |= CF;
        if((*a > INT16_MAX - *b) || (*a < INT16_MIN - *b)) FLG |= OF;
        if(parchk16(sum)) FLG |= PF;
        if(!d) *a = sum;
        else *b = sum;
        r->ip += 2;
        return;
      } else {
        int8_t *a = (int8_t *)regtor8(rm, r);
        int8_t *b = (int8_t *)regtor8(reg, r);
        if(!d) *a += cstate;
        else *b += cstate;
        int8_t sum = *a + *b;
        if(sum < 0) FLG |= SF;
        if(sum == 0) FLG |= ZF;
        if(((uint16_t)*a + (uint16_t)*b) > UINT8_MAX) FLG |= CF;
        if((*a > INT8_MAX - *b) || (*a < INT8_MIN - *b)) FLG |= OF;
        if(parchk8(sum)) FLG |= PF;
        if(!d) *a = sum;
        else *b = sum;
        r->ip += 2;
        return;
      }
    } else {
      uint16_t disp = 0;
      if(mod == 0b01) disp = mem[a_seg(r->ip + 2, SR_CS, r)];
      if((mod == 2) || ((mod == 0) && (rm == 0x06))) disp = fetch16(a_seg(r->ip + 2, SR_CS, r), mem);
      if(w) {
        int16_t *a = (int16_t *)&mem[ea_calc(rm, mod, disp, r, s_ovrd)];
        int16_t *b = (int16_t *)regtor16(reg, r);
        if(!d) *a += cstate;
        else *b += cstate;
        int16_t sum = *a + *b;
        if(sum < 0) FLG |= SF;
        if(sum == 0) FLG |= ZF;
        if(((uint32_t)*a + (uint32_t)*b) > UINT16_MAX) FLG |= CF;
        if((*a > INT16_MAX - *b) || (*a < INT16_MIN - *b)) FLG |= OF;
        if(parchk16(sum)) FLG |= PF;
        if(!d) *a = sum;
        else *b = sum;
      } else {
        int8_t *a = (int8_t *)&mem[ea_calc(rm, mod, disp, r, s_ovrd)];
        int8_t *b = (int8_t *)regtor8(reg, r);
        if(!d) *a += cstate;
        else *b += cstate;
        int8_t sum = *a + *b;
        if(sum < 0) FLG |= SF;
        if(sum == 0) FLG |= ZF;
        if(((uint16_t)*a + (uint16_t)*b) > UINT8_MAX) FLG |= CF;
        if((*a > INT8_MAX - *b) || (*a < INT8_MIN - *b)) FLG |= OF;
        if(parchk8(sum)) FLG |= PF;
        if(!d) *a = sum;
        else *b = sum;
      }

      r->ip += 2 + mod;
      if((mod == 0) && (rm == 0x06)) r->ip += 2;
      return;
    }
  }

  if((opc) >> 2 == 0b100000) {
    uint8_t f = mem[a_seg(r->ip + 1, SR_CS, r)];
    if(((f >> 3) & 0b111) != 0b010) return;
    uint8_t rm = f & 0b111;
    uint8_t mod = f >> 6;
    uint8_t sw = opc & 0b11;
    *(uint16_t *)r->flg &= ~(SF | ZF | CF | PF | OF);
    if(mod == 0b11) {
      if(sw & 1) {
        int16_t *a = (int16_t *)regtor16(rm, r);
        int16_t b = 0;
        if(sw == 0b01) b = (int16_t)mem[a_seg(r->ip + 2, SR_CS, r)];
        if(sw == 0b11) b = (int16_t)((int8_t *)mem)[a_seg(r->ip + 2, SR_CS, r)];
        b += cstate;
        int16_t sum = *a + b;
        if(sum < 0) FLG |= SF;
        if(sum == 0) FLG |= ZF;
        if(((uint32_t)*a + (uint32_t)b) > UINT16_MAX) FLG |= CF;
        if((*a > INT16_MAX - b) || (*a < INT16_MIN - b)) FLG |= OF;
        if(parchk16(sum)) FLG |= PF;
        *a = sum;
        r->ip += 2;
        if(sw == 0b01) r->ip += 2;
        if(sw == 0b11) r->ip += 1;
        return;
      } else {
        int8_t *a = (int8_t *)regtor8(rm, r);
        int8_t b = (int8_t)mem[a_seg(r->ip + 2, SR_CS, r)];
        b += cstate;
        int8_t sum = *a + b;
        if(sum < 0) FLG |= SF;
        if(sum == 0) FLG |= ZF;
        if(((uint32_t)*a + (uint32_t)b) > UINT8_MAX) FLG |= CF;
        if((*a > INT8_MAX - b) || (*a < INT8_MIN - b)) FLG |= OF;
        if(parchk8(sum)) FLG |= PF;
        *a = sum;
        r->ip += 3;
        return;
      }
    } else {
      uint16_t disp = 0;
      if(mod == 0b01) disp = mem[a_seg(r->ip + 2, SR_CS, r)];
      if((mod == 2) || ((mod == 0) && (rm == 0x06))) disp = fetch16(a_seg(r->ip + 2, SR_CS, r), mem);
      if(sw & 1) {
        int16_t *a = (int16_t *)&mem[ea_calc(rm, mod, disp, r, s_ovrd)];
        int16_t b = 0;
        r->ip += 2 + mod;
        if((mod == 0) && (rm == 0x6)) r->ip += 2;
        if(sw == 0b01) b = *(int16_t *)&mem[a_seg(r->ip, SR_CS, r)];
        if(sw == 0b11) b = (int16_t)((int8_t *)mem)[a_seg(r->ip, SR_CS, r)];
        b += cstate;
        int16_t sum = *a + b;
        if(sum < 0) FLG |= SF;
        if(sum == 0) FLG |= ZF;
        if(((uint32_t)*a + (uint32_t)b) > UINT16_MAX) FLG |= CF;
        if((*a > INT16_MAX - b) || (*a < INT16_MIN - b)) FLG |= OF;
        if(parchk16(sum)) FLG |= PF;
        *a = sum;
        if(sw == 0b01) r->ip += 2;
        if(sw == 0b11) r->ip += 1;
        return;
      } else {
        int8_t *a = (int8_t *)&mem[ea_calc(rm, mod, disp, r, s_ovrd)];
        r->ip += 2 + mod;
        if((mod == 0) && (rm == 0x6)) r->ip += 2;
        int8_t b = (int8_t)mem[a_seg(r->ip, SR_CS, r)];
        b += cstate;
        int8_t sum = *a + b;
        if(sum < 0) FLG |= SF;
        if(sum == 0) FLG |= ZF;
        if(((uint32_t)*a + (uint32_t)b) > UINT8_MAX) FLG |= CF;
        if((*a > INT8_MAX - b) || (*a < INT8_MIN - b)) FLG |= OF;
        if(parchk8(sum)) FLG |= PF;
        *a = sum;
        return;
      }
    }
  }

  if((opc >> 1) == 0b0001010) {
    uint8_t w = opc & 1;
    if(w) {
      int16_t *a = (int16_t *)r->ax;
      int16_t *b = (int16_t *)&mem[a_seg(r->ip + 1, SR_CS, r)];
      *a += cstate;
      int16_t sum = *a + *b;
      if(sum < 0) FLG |= SF;
      if(sum == 0) FLG |= ZF;
      if(((uint32_t)*a + (uint32_t)*b) > UINT16_MAX) FLG |= CF;
      if((*a > INT16_MAX - *b) || (*a < INT16_MIN - *b)) FLG |= OF;
      if(parchk16(sum)) FLG |= PF;
      *a = sum;
      r->ip += 3;
      return;
    } else {
      int8_t *a = (int8_t *)&r->ax[0];
      int8_t *b = (int8_t *)&mem[a_seg(r->ip + 1, SR_CS, r)];
      *a += cstate;
      int8_t sum = *a + *b;
      if(sum < 0) FLG |= SF;
      if(sum == 0) FLG |= ZF;
      if(((uint32_t)*a + (uint32_t)*b) > UINT8_MAX) FLG |= CF;
      if((*a > INT8_MAX - *b) || (*a < INT8_MIN - *b)) FLG |= OF;
      if(parchk8(sum)) FLG |= PF;
      *a = sum;
      r->ip += 2;
      return;
    }
  }
}
 
void inc(reg_t *r, uint8_t *mem) {
  uint8_t opc = CURBYTE;
  if((opc >> 1) == 0b1111111) {
    uint8_t f = NXTBYTE;
    if(((f >> 3) & 0b111) != 0b000) return;
    uint8_t w = opc & 1;
    uint8_t rm = f & 0b111;
    uint8_t mod = f >> 6;
    if(mod == 0b11) {
      if(w) {
        uint16_t *a = regtor16(rm, r);
        if(*a == 0xffff) FLG |= (OF | ZF);
        (*a)++;
        if((int16_t)*a > 0) FLG |= SF;
        if(parchk16(*a)) FLG |= PF;
        r->ip += 2;
        return;
      } else {
        uint8_t *a = regtor8(rm, r);
        if(*a == 0xff) FLG |= (OF | ZF);
        (*a)++;
        if((int8_t)*a > 0) FLG |= SF;
        if(parchk8(*a)) FLG |= PF;
        r->ip += 2;
        return;
      }
    } else {
      uint16_t disp = 0;
      if(mod == 0b01) {
        disp = mem[a_seg(r->ip + 2, SR_CS, r)];
        r->ip += 3;
      }
      if((mod == 0b10) || ((mod == 0) && (rm == 0x06))) {
        disp = fetch16(a_seg(r->ip + 2, SR_CS, r), mem);
        r->ip += 4;
      }
      if(w) {
        uint16_t *a = (uint16_t *)&mem[ea_calc(rm, mod, disp, r, s_ovrd)];
        if(*a == 0xffff) FLG |= (OF | ZF);
        (*a)++;
        if((int16_t)*a > 0) FLG |= SF;
        if(parchk16(*a)) FLG |= PF;
        return;
      } else {
        uint8_t *a = (uint8_t *)&mem[ea_calc(rm, mod, disp, r, s_ovrd)];
        if(*a == 0xff) FLG |= (OF | ZF);
        (*a)++;
        if((int8_t)*a > 0) FLG |= SF;
        if(parchk8(*a)) FLG |= PF;
        return;
      }
    }
  }

  if((opc >> 3) == 0b01000) {
    uint8_t reg = opc & 0b111;
    uint16_t *a = regtor16(reg, r);
    if(*a == 0xffff) FLG |= (OF | ZF);
    (*a)++;
    if((int16_t)*a > 0) FLG |= SF;
    if(parchk16(*a)) FLG |= PF;
    r->ip += 1;
    return;
  }
}
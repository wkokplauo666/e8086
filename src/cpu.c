#include "cpu.h"

#include <stdio.h>
#include <stdlib.h>

#include "ins.h"
#include "mem.h"

reg_t g_reg = { 0 };
simflg_t g_simflg = 0;
uint8_t s_ovrd;



int init() {
  mem = malloc(1 << 20);
  if(!mem) {
    perror("cpuinit: malloc");
    return 1;
  }
  printf("memory initialized\n");
  g_simflg |= MEM_INIT;

  g_reg = (reg_t){ 0 };
  s_ovrd = SR_DF;

  g_reg.cs = 0xf000;

  g_simflg |= CPU_INIT;
  return 0;
}

void step() {
  uint8_t opc = mem[a_seg(g_reg.ip, SR_CS, &g_reg)];
  uint8_t inv = 0;
  if(((opc & 0b111) == 0b110) && ((opc >> 5) == 0b001)) {
    // printf("segment override prefix\n");
    s_ovrd = (opc >> 3) & 0b11;
    g_reg.ip++;
    step();
    s_ovrd = SR_DF;
    return;
  }

  switch(opc) {  
  case 0b10001000:
  case 0b10001001:
  case 0b10001010:
  case 0b10001011:
    move  (&g_reg, mem);
    break;
  case 0b11000110:
  case 0b11000111:
    move  (&g_reg, mem);
    break;
  case 0b10110000:
  case 0b10110001:
  case 0b10110010:
  case 0b10110011:
  case 0b10110100:
  case 0b10110101:
  case 0b10110110:
  case 0b10110111:
  case 0b10111000:
  case 0b10111001:
  case 0b10111010:
  case 0b10111011:
  case 0b10111100:
  case 0b10111101:
  case 0b10111110:
  case 0b10111111:
    move  (&g_reg, mem);
    break;
  case 0b10100000:
  case 0b10100001:
  case 0b10100010:
  case 0b10100011:
    move  (&g_reg, mem);
    break;
  case 0b10001110:
  case 0b10001100:
    move  (&g_reg, mem);
    break;
  case 0b11111110:
    inc   (&g_reg, mem);
    break;
  case 0b11111111:
    push  (&g_reg, mem);
    jmp   (&g_reg, mem);
    call  (&g_reg, mem);
    inc   (&g_reg, mem);
    break;
  case 0b01010000:
  case 0b01010001:
  case 0b01010010:
  case 0b01010011:
  case 0b01010100:
  case 0b01010101:
  case 0b01010110:
  case 0b01010111:
    push  (&g_reg, mem);
    break;
  case 0b00000110:
  case 0b00001110:
  case 0b00010110:
  case 0b00011110:
    push  (&g_reg, mem);
    break;
  case 0b10001111:
    pop   (&g_reg, mem);
    break;
  case 0b01011000:
  case 0b01011001:
  case 0b01011010:
  case 0b01011011:
  case 0b01011100:
  case 0b01011101:
  case 0b01011110:
  case 0b01011111:
    pop   (&g_reg, mem);
    break;
  case 0b00000111:
  case 0b00001111:
  case 0b00010111:
  case 0b00011111:
    pop   (&g_reg, mem);
    break;
  case 0b00111000:
  case 0b00111001:
  case 0b00111010:
  case 0b00111011:
    cmp   (&g_reg, mem);
    break;
  case 0b10000000:
  case 0b10000001:
  case 0b10000010:
  case 0b10000011:
    cmp   (&g_reg, mem);
    add   (&g_reg, mem);
    adc   (&g_reg, mem);
    break;
  case 0b00111100:
  case 0b00111101:
    cmp   (&g_reg, mem);
    break;
  case 0b11001101:
  case 0b11001100:
    intr  (&g_reg, mem);
    break;
  case 0b11001111:
  case 0b11001110:
    intr  (&g_reg, mem);
    break;
  case 0b11101001:
  case 0b11101011:
  case 0b11101010:
    jmp   (&g_reg, mem);
    break;
  case 0b11101000:
  case 0b10011010:
    call  (&g_reg, mem);
    break;
  case 0b11000011:
  case 0b11000010:
    ret   (&g_reg, mem);
    break;
  case 0b11001011:
  case 0b11001010:
    ret   (&g_reg, mem);
    break;
  case 0b01110100:
  case 0b01111100:
  case 0b01111110:
  case 0b01110010:
  case 0b01110110:
  case 0b01111010:
  case 0b01110000:
  case 0b01111000:
  case 0b01110101:
  case 0b01111101:
  case 0b01111111:
  case 0b01110011:
  case 0b01110111:
  case 0b01111011:
  case 0b01110001:
  case 0b01111001:
  case 0b11100011:
    jcc   (&g_reg, mem);
    break;
  case 0b11100010:
  case 0b11100001:
  case 0b11100000:
    loop  (&g_reg, mem);
    break;
  case 0b11100100:
  case 0b11100101:
  case 0b11101100:
  case 0b11101101:
    inout (&g_reg, mem);
    break;
  case 0b11100110:
  case 0b11100111:
  case 0b11101110:
  case 0b11101111:
    inout (&g_reg, mem);
    break;
  case 0b11111000:
  case 0b11110101:
  case 0b11111001:
  case 0b11111100:
  case 0b11111101:
  case 0b11111010:
  case 0b11111011:
  case 0b11110100:
  case 0b10011011:
  case 0b11110000:
    pctrl (&g_reg, mem);
    break;
  case 0b10000110:
  case 0b10000111:
    xchg  (&g_reg, mem);
    break;
  case 0b10010000:
  case 0b10010001:
  case 0b10010010:
  case 0b10010011:
  case 0b10010100:
  case 0b10010101:
  case 0b10010110:
  case 0b10010111:
    xchg  (&g_reg, mem);
    break;
  case 0b11010111:
    xlat  (&g_reg, mem);
    break;
  case 0b10001101:
    lea   (&g_reg, mem);
    break;
  case 0b11000101:
    lds   (&g_reg, mem);
    break;
  case 0b11000100:
    les   (&g_reg, mem);
    break;
  case 0b10011111:
  case 0b10011110:
  case 0b10011100:
  case 0b10011101:
    lsahf (&g_reg, mem);
    break;
  case 0b00000000:
  case 0b00000001:
  case 0b00000010:
  case 0b00000011:
  case 0b00000100:
  case 0b00000101:
    add   (&g_reg, mem);
    break;
  case 0b00010000:
  case 0b00010001:
  case 0b00010010:
  case 0b00010011:
  case 0b00010100:
  case 0b00010101:
    adc   (&g_reg, mem);
    break;
  case 0b01000000:
  case 0b01000001:
  case 0b01000010:
  case 0b01000011:
  case 0b01000100:
  case 0b01000101:
  case 0b01000110:
  case 0b01000111:
    inc   (&g_reg, mem);
    break;
  default:
    inv = 10;
    break;
  }

  if(inv >= 6) {
    printf("invalid instruction 0x%02x\n", opc);
    return;
  }

  return;
}

void dump(const uint8_t *data, uint16_t len, uint32_t offset, uint32_t haddr) {
  const uint16_t bytes_per_line = 16;
  if(haddr < offset) haddr = offset;
  for(uint32_t i = 0; i < len; i += bytes_per_line) {
    printf("%08x:  ", offset + i);

    for(uint32_t j = 0; j < bytes_per_line; j++) {
      if((i + j < len) && (i + j) != (haddr - offset)) {
        printf("%02x ", data[i + j + offset]);
      } else if((i + j) == (haddr - offset)) {
        printf("\033[7m%02x\033[0m ", data[i + j + offset]);
      } else {
        printf("   ");
      }

      if(j == (uint32_t)bytes_per_line / 2 - 1) {
        printf(" ");
      }
    }

    printf(" |");
    for(uint16_t j = 0; j < bytes_per_line; j++) {
      if(i + j < len) {
        uint8_t c = data[i + j + offset];
        printf("%c", isprint(c) ? c : '.');
      } else {
        printf(" ");
      }
    }
    printf("|\n");
  }
}

void print_flg(uint16_t flg) {
  if(flg & CF) {
    printf("CF ");
  }

  if(flg & PF) {
    printf("PF ");
  }

  if(flg & AF) {
    printf("AF ");
  }

  if(flg & ZF) {
    printf("ZF ");
  }

  if(flg & SF) {
    printf("SF ");
  }

  if(flg & TF) {
    printf("TF ");
  }

  if(flg & IF) {
    printf("IF ");
  }

  if(flg & DF) {
    printf("DF ");
  }

  if(flg & OF) {
    printf("OF ");
  }
  printf("\n");
  return;
}

uint32_t offset(uint32_t snap, uint32_t addr) {
  if(addr % snap == 0) {
    return addr;
  } else {
    return addr - (addr % snap);
  }
}

void print_dbg(reg_t *r, uint8_t *m) {

  printf("AX: 0x%04x\n", *(uint16_t *)r->ax);
  printf("BX: 0x%04x\n", *(uint16_t *)r->bx);
  printf("CX: 0x%04x\n", *(uint16_t *)r->cx);
  printf("DX: 0x%04x\n\n", *(uint16_t *)r->dx);

  printf("IP: 0x%04x\n", r->ip);
  printf("SP: 0x%04x\n", r->sp);
  printf("BP: 0x%04x\n", r->bp);
  printf("SI: 0x%04x\n", r->si);
  printf("DI: 0x%04x\n\n", r->di);

  printf("CS: 0x%04x\n", r->cs);
  printf("DS: 0x%04x\n", r->ds);
  printf("SS: 0x%04x\n", r->ss);
  printf("ES: 0x%04x\n\n", r->es);

  uint16_t flg = *(uint16_t *)r->flg;
  printf("FLG: 0x%04x:  ", flg);
  print_flg(flg);
  printf("\n");

  if(a_seg(r->sp, SR_SS, r) >= 8) {
    printf("stack\n");
    dump(m, 16, offset(8, a_seg(r->sp, SR_SS, r)), a_seg(r->sp, SR_SS, r));
  }

  printf("IP 2\n");
  dump(m, 16, offset(8, a_seg(r->ip, SR_CS, r)), a_seg(r->ip, SR_CS, r));
}

uint8_t parchk16(uint16_t x) {
  x = (uint8_t) x;
  x ^= x >> 4;
  x ^= x >> 2;
  x ^= x >> 1;
  return x & 1;
}

uint8_t parchk8(uint8_t x) {
  x ^= x >> 4;
  x ^= x >> 2;
  x ^= x >> 1;
  return x & 1;
}

uint8_t *regtor8(uint8_t reg, reg_t *r) {
  switch(reg) {
  case 0x00:
    return (uint8_t *)&r->ax[0];
    break;
  case 0x01:
    return (uint8_t *)&r->cx[0];
    break;
  case 0x02:
    return (uint8_t *)&r->dx[0];
    break;
  case 0x03:
    return (uint8_t *)&r->bx[0];
    break;
  case 0x04:
    return (uint8_t *)&r->ax[1];
    break;
  case 0x05:
    return (uint8_t *)&r->cx[1];
    break;
  case 0x06:
    return (uint8_t *)&r->dx[1];
    break;
  case 0x07:
    return (uint8_t *)&r->bx[1];
  default:
    printf("WARN: attemp to access an invalid register was made (regtor8)\n");
    break;
  }
  return (uint8_t *)&r->ax[0];
}

uint16_t *regtor16(uint8_t reg, reg_t *r) {
  switch(reg) {
  case 0x00:
    return (uint16_t *)r->ax;
    break;
  case 0x01:
    return (uint16_t *)r->cx;
    break;
  case 0x02:
    return (uint16_t *)r->dx;
    break;
  case 0x03:
    return (uint16_t *)r->bx;
    break;
  case 0x04:
    return (uint16_t *)&r->sp;
    break;
  case 0x05:
    return (uint16_t *)&r->bp;
    break;
  case 0x06:
    return (uint16_t *)&r->si;
    break;
  case 0x07:
    return (uint16_t *)&r->di;
    break;
  default:
    printf("WARN: attemp to access an invalid register was made (regtor16)\n");
    break;
  }
  return (uint16_t *)r->ax;
}

uint16_t *regtos16(uint8_t reg, reg_t *r) {
  switch(reg) {
  case 0x00:
    return &r->es;
    break;
  case 0x01:
    return &r->cs;
    break;
  case 0x02:
    return &r->ss;
    break;
  case 0x03:
    return &r->ds;
    break;
  default:
    printf("WARN: attemp to access an invalid register was made (regtos16)\n");
    break;
  }
  return (uint16_t *)&r->es;
}

void intrt_h(uint8_t type, reg_t *r, uint8_t *mem, uint16_t ra) {
  set16(*(uint16_t*)r->flg, a_seg(r->sp - 2, SR_SS, r), mem);
  r->sp -= 2;
  set16(r->cs, a_seg(r->sp - 2, SR_SS, r), mem);
  r->sp -= 2;
  set16(ra, a_seg(r->sp - 2, SR_SS, r), mem);
  r->sp -= 2;
  r->ip = fetch16(type * 4, mem);
  r->cs = fetch16(type * 4 + 2, mem);
  *(uint16_t*)r->flg &= ~(IF | TF);
}

void handleout(uint16_t port, uint16_t data, uint8_t word) {
  //printf("out at %04x with %04x\n", port, data);
  (void)port;
  if(!word) {
    putc((int)data, stdout);
  }
  return;
}

uint16_t handlein(uint16_t port, uint8_t word) {
  //printf("in at %04x\n", port);
  (void)port;
  if(!word) {
    printf("\033[2 q");
    uint16_t a = getc(stdin);
    printf("\033[3 q");
    return a;
  }
  return 0;
}
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <getopt.h>

#include "cpu.h"
#include "ins.h"
#include "mem.h"

struct termios orig;

void unbuf(void) {
  struct termios t;

  tcgetattr(STDIN_FILENO, &orig);
  memcpy(&t, &orig, sizeof(struct termios));

  t.c_lflag &= ~(ICANON | ECHO);

  tcsetattr(STDIN_FILENO, TCSANOW, &t);

  g_simflg |= TRM_UNBF;
}

void buf() {
  tcsetattr(STDIN_FILENO, TCSANOW, &orig);
}

void atexith(void) {
  if(g_simflg & MEM_INIT) {
    free(mem);
    printf("free'd memory\n");
  };

  if(g_simflg & TRM_UNBF) {
    buf();
    printf("terminal buffered\n");
  }

  printf("exiting...\n");
}

void siginth(int sig) {
  (void)sig;
  exit(1);
  return;
}

int main(int argc, char **argv) {
  int opt;
  while((opt = getopt(argc, argv, "hd")) != -1) {
    switch(opt) {
    case 'h':
      break;
    case 'd':
      g_simflg |= SIM_DEBG;
      break;
    case '?':
      printf("unknown option -%c\n", optopt);
      return 2;
      break;
    default:
      printf("unknown option -%c\n", optopt);
      return 2;
      break;
    }
  }

  if((argc - optind) < 1) return 2;

  atexit(atexith);
  signal(SIGINT, siginth);
  if(init()) {
    fprintf(stderr, "failed initializing\n");
    return 1;
  }
  printf("cpu initialized\n");
  unbuf();
  printf("terminal unbuffered\n");

  FILE *f = fopen(argv[optind], "rb");
  if(!f) {
    perror("fopen");
    return 1;
  }
  fseek(f, 0, SEEK_END);
  long fsiz = ftell(f);
  fseek(f, 0, SEEK_SET);
  fread(mem + 0xf0000, 1, fsiz, f);
  fclose(f);
  
  setvbuf(stdout, NULL, _IONBF, 0);
  print_dbg(&g_reg, mem);

  if(g_simflg & SIM_DEBG) {
    while(1) {
      int c;
      if((c = getchar())) {
        switch(c) {
          case 'd': {
          printf("d");
          buf();
          uint16_t addr = 0, segment = 0;
          scanf("%04hx:%04hx", &segment, &addr);
          dump(mem, 128, (segment << 4) + addr, 0);
          unbuf();
          while(getchar() != 'd');
          break;
          }
        }
      }
      step();
      print_dbg(&g_reg, mem);
    }
  } else {
    while(1) {
      step();
    }
  }

  return 0;
}
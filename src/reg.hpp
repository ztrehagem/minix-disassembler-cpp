#pragma once

union SharedReg {
  unsigned x:16;
  struct {
    unsigned h:8;
    unsigned l:8;
  } hl;

  SharedReg() {
    x = 0x00;
  }
};

struct Reg {
  union SharedReg a;
  union SharedReg b;
  union SharedReg c;
  union SharedReg d;
  unsigned sp:16;
  unsigned bp:16;
  unsigned si:16;
  unsigned di:16;

  Reg() {
    sp = 0x00;
    bp = 0x00;
    si = 0x00;
    di = 0x00;
  }
};

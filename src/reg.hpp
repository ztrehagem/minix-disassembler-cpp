#pragma once

#include <string>
#include <sstream>
#include "util.hpp"

union SharedReg {
  unsigned short x;
  struct {
    unsigned char h;
    unsigned char l;
  } hl;

  SharedReg() {
    x = 0x0000;
  }
};

struct Reg {
  union SharedReg a;
  union SharedReg b;
  union SharedReg c;
  union SharedReg d;
  unsigned short sp;
  unsigned short bp;
  unsigned short si;
  unsigned short di;

  Reg() {
    sp = 0xffdc;//0x00;
    bp = 0x0000;
    si = 0x0000;
    di = 0x0000;
  }

  std::string str() {
    std::ostringstream ss;
    ss << util::hex_str(a.x & 0xffff, 4) << ' ';
    ss << util::hex_str(b.x & 0xffff, 4) << ' ';
    ss << util::hex_str(c.x & 0xffff, 4) << ' ';
    ss << util::hex_str(d.x & 0xffff, 4) << ' ';
    ss << util::hex_str(sp & 0xffff, 4) << ' ';
    ss << util::hex_str(bp & 0xffff, 4) << ' ';
    ss << util::hex_str(si & 0xffff, 4) << ' ';
    ss << util::hex_str(di & 0xffff, 4);
    return ss.str();
  }
};

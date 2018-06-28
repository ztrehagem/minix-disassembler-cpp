#pragma once

#include <string>
#include <sstream>

struct Flags {
  bool o, d, i, t, s, z, a, p, c;

  Flags() {
    o = false; // overflow
    d = false;
    i = false;
    t = false;
    s = false; // sine (neg = true)
    z = false; // zero (0 = true)
    a = false;
    p = false;
    c = false; // carry
  }

  std::string str() {
    std::ostringstream ss;
    ss << (o ? 'O' : '-');
    ss << (s ? 'S' : '-');
    ss << (z ? 'Z' : '-');
    ss << (c ? 'C' : '-');
    return ss.str();
  }
};
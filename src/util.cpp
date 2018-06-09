#include <string>
#include <sstream>
#include <iomanip>
#include "util.hpp"

unsigned short util::get_data_wide(const char *head) {
  return (head[0] & 0xff) + ((head[1] & 0xff) << 8);
}

unsigned char util::get_data_narrow(const char *head) {
  return head[0] & 0xff;
}

size_t util::get_extended_len(const Inst &inst) {
  switch (inst.mod) {
    case 0b10: return 2;
    case 0b01: return 1;
    case 0b00: return inst.rm == 0b110 ? 2 : 0;
    default: return 0;
  }
}

string util::data_str_wide(const unsigned short data, const bool nat, const bool sign) {
  return hex_str(sign ? static_cast<short>(data & 0xffff) : (data & 0xffff), nat ? 0 : 4);
}

string util::data_str_narrow(const unsigned char data, const bool nat, const bool sign) {
  return hex_str(sign ? static_cast<char>(data & 0xff) : (data & 0xff), nat ? 0 : 2);
}

string util::line_number_str(const size_t n) {
  return hex_str(n & 0xffff, 4) + ": ";
}

string util::reg_state_str(struct Reg &reg) {
  ostringstream ss;
  ss << hex_str(reg.a.x & 0xff, 4) << ' ';
  ss << hex_str(reg.b.x & 0xff, 4) << ' ';
  ss << hex_str(reg.c.x & 0xff, 4) << ' ';
  ss << hex_str(reg.d.x & 0xff, 4) << ' ';
  ss << hex_str(reg.sp & 0xff, 4) << ' ';
  ss << hex_str(reg.bp & 0xff, 4) << ' ';
  ss << hex_str(reg.si & 0xff, 4) << ' ';
  ss << hex_str(reg.di & 0xff, 4) << ' ';
  return ss.str();
}

string util::instruction_str(const char *head, const size_t len) {
  ostringstream ss;
  size_t i = 0;
  for (; i < len; i++) ss << hex_str(head[i] & 0xff, 2);
  for (; i < 7; i++) ss << "  ";
  return ss.str();
}

string util::hex_str(int value, size_t w) {
  ostringstream ss;
  if (value < 0) {
    ss << '-';
    value = -value;
  }
  if (w > 0) {
    ss << setfill('0') << setw(w) << hex << value;
  } else {
    ss << hex << value;
  }
  return ss.str();
}

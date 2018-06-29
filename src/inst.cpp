#include <iostream>
#include <sstream>
#include "inst.hpp"
#include "util.hpp"

using namespace std;

void Inst::set_mod_sec() {
  has_mod_sec = true;
  mod = *machine.get_head(1) >> 6;
  reg = *machine.get_head(1) >> 3;
  rm = *machine.get_head(1);

  if (mod == 0b10 || (mod == 0b00 && rm == 0b110)) {
    disp = static_cast<signed short>(util::get_data_wide(machine.get_head(2)));
    disp_size = 2;
  } else if (mod == 0b01) {
    disp = static_cast<signed char>(util::get_data_narrow(machine.get_head(2)));
    disp_size = 1;
  }
}

void Inst::set_data() {
  const size_t offset = 1 + has_mod_sec + disp_size;
  has_data_sec = true;
  if (is_wide_data()) {
    data.wide = util::get_data_wide(machine.get_head(offset));
    data_size = 2;
  } else {
    data.narrow = util::get_data_narrow(machine.get_head(offset));
    data_size = 1;
  }
}

bool Inst::is_wide_data() {
  return s == 0 && w == 1;
}

size_t Inst::get_inst_len() {
  return 1 + has_mod_sec + disp_size + data_size;
}

string Inst::get_inst_str(const char *op) {
  return util::instruction_str(machine.get_head(), get_inst_len()) + op + ' ';
}

string Inst::get_reg_name(const bool is_rm) {
  switch (is_rm ? rm : reg) {
    case 0b000: return w ? "ax" : "al";
    case 0b001: return w ? "cx" : "cl";
    case 0b010: return w ? "dx" : "dl";
    case 0b011: return w ? "bx" : "bl";
    case 0b100: return w ? "sp" : "ah";
    case 0b101: return w ? "bp" : "ch";
    case 0b110: return w ? "si" : "dh";
    case 0b111: return w ? "di" : "bh";
    default: return "";
  }
}

string Inst::get_rm_str() {
  if (mod == 0b11) {
    return get_reg_name(true);
  }

  if (mod == 0b00 && rm == 0b110) {
    return "[" + util::data_str_wide(disp) + "]";
  }

  ostringstream oss;

  switch (rm & 0b111) {
    case 0b000: oss << "bx+si"; break;
    case 0b001: oss << "bx+di"; break;
    case 0b010: oss << "bp+si"; break;
    case 0b011: oss << "bp+di"; break;
    case 0b100: oss << "si"; break;
    case 0b101: oss << "di"; break;
    case 0b110: oss << "bp"; break;
    case 0b111: oss << "bx"; break;
  }

  if (disp > 0) {
    oss << "+" << hex << disp;
  } else if (disp < 0) {
    oss << "-" << hex << -disp;
  }

  return "[" + oss.str() + "]";
}

string Inst::get_data_str(const bool sign) {
  if (is_wide_data()) {
    return util::data_str_wide(data.wide, false);
  } else {
    return util::data_str_narrow(data.narrow, true, sign);
  }
}

string Inst::get_dist_str() {
  ostringstream oss;
  if (d) {
    oss << get_reg_name() << ", " << get_rm_str();
  } else {
    oss << get_rm_str() << ", " << get_reg_name();
  }
  return oss.str();
}

string Inst::get_accumulator_str() {
  return w ? "ax" : "al";
}

int Inst::get_data_value(bool sign) {
  if (is_wide_data()) {
    return sign ? static_cast<signed short>(data.wide) : data.wide;
  } else {
    return sign ? static_cast<signed char>(data.narrow) : data.narrow;
  }
}

int Inst::get_reg_value(bool sign, bool is_rm) {
  Reg &r = machine.get_reg();

  switch ((is_rm ? rm : reg) + (w << 3)) {
    case 0b1000: return sign ? static_cast<signed short>(r.a.x) : r.a.x;
    case 0b0000: return sign ? static_cast<signed char>(r.a.hl.l) : r.a.hl.l;
    case 0b1001: return sign ? static_cast<signed short>(r.c.x) : r.c.x;
    case 0b0001: return sign ? static_cast<signed char>(r.c.hl.l) : r.c.hl.l;
    case 0b1010: return sign ? static_cast<signed short>(r.d.x) : r.d.x;
    case 0b0010: return sign ? static_cast<signed char>(r.d.hl.l) : r.d.hl.l;
    case 0b1011: return sign ? static_cast<signed short>(r.b.x) : r.b.x;
    case 0b0011: return sign ? static_cast<signed char>(r.b.hl.l) : r.b.hl.l;
    case 0b1100: return sign ? static_cast<signed short>(r.sp) : r.sp;
    case 0b0100: return sign ? static_cast<signed char>(r.a.hl.h) : r.a.hl.h;
    case 0b1101: return sign ? static_cast<signed short>(r.bp) : r.bp;
    case 0b0101: return sign ? static_cast<signed char>(r.c.hl.h) : r.c.hl.h;
    case 0b1110: return sign ? static_cast<signed short>(r.si) : r.si;
    case 0b0110: return sign ? static_cast<signed char>(r.d.hl.h) : r.d.hl.h;
    case 0b1111: return sign ? static_cast<signed short>(r.di) : r.di;
    case 0b0111: return sign ? static_cast<signed char>(r.b.hl.h) : r.b.hl.h;
  }
  return 0;
}

int Inst::get_ea_value() {
  Reg &r = machine.get_reg();
  int ea;

  switch (rm & 0b111) {
    case 0b000: ea = r.b.x + r.si; break;
    case 0b001: ea = r.b.x + r.di; break;
    case 0b010: ea = r.bp + r.si; break;
    case 0b011: ea = r.bp + r.di; break;
    case 0b100: ea = r.si; break;
    case 0b101: ea = r.di; break;
    case 0b110: ea = r.bp; break;
    case 0b111: ea = r.b.x; break;
  }

  ea += disp;

  return ea;
}

int Inst::get_rm_value(bool sign) {
  if (mod == 0b11) {
    return get_reg_value(sign, true);
  }

  char *data_seg = machine.get_data_seg();

  if (mod == 0b00 && rm == 0b110) {
    return sign ? static_cast<signed char>(data_seg[disp]) : data_seg[disp];
  }

  int ea = get_ea_value();

  cout << "; (EA:" << hex << ea << "=" << (int)data_seg[ea] << ")";

  return sign ? static_cast<signed char>(data_seg[ea]) : data_seg[ea];
}

int Inst::get_accum_value() {
  Reg &r = machine.get_reg();
  return is_wide_data() ? r.a.x : r.a.hl.l;
}

void Inst::put_value_reg(const unsigned int value, bool is_rm) {
  Reg &r = machine.get_reg();

  switch ((is_rm ? rm : reg) + (w << 3)) {
    case 0b1000: r.a.x = value; break;
    case 0b0000: r.a.hl.l = value; break;
    case 0b1001: r.c.x = value; break;
    case 0b0001: r.c.hl.l = value; break;
    case 0b1010: r.d.x = value; break;
    case 0b0010: r.d.hl.l = value; break;
    case 0b1011: r.b.x = value; break;
    case 0b0011: r.b.hl.l = value; break;
    case 0b1100: r.sp = value; break;
    case 0b0100: r.a.hl.h = value; break;
    case 0b1101: r.bp = value; break;
    case 0b0101: r.c.hl.h = value; break;
    case 0b1110: r.si = value; break;
    case 0b0110: r.d.hl.h = value; break;
    case 0b1111: r.di = value; break;
    case 0b0111: r.b.hl.h = value; break;
  }
}

void Inst::put_value_rm(const unsigned int value) {
  if (mod == 0b11) {
    put_value_reg(value, true);
    return;
  }

  char *data_seg = machine.get_data_seg();

  if (mod == 0b00 && rm == 0b110) {
    data_seg[disp] = value;
    return;
  }

  int ea = get_ea_value();

  data_seg[ea] = value;
}

void Inst::put_accum_value(const unsigned int value) {
  Reg &r = machine.get_reg();
  if (is_wide_data()) {
    r.a.x = value;
  } else {
    r.a.hl.l = value;
  }
}
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

int Inst::get_data_value() {
  if (is_wide_data()) {
    return data.wide;
  } else {
    return data.narrow;
  }
}

int Inst::get_reg_value(bool is_rm) {
  switch ((is_rm ? rm : reg) + (w << 3)) {
    case 0b1000: return machine.reg.a.x;
    case 0b0000: return machine.reg.a.hl.l;
    case 0b1001: return machine.reg.c.x;
    case 0b0001: return machine.reg.c.hl.l;
    case 0b1010: return machine.reg.d.x;
    case 0b0010: return machine.reg.d.hl.l;
    case 0b1011: return machine.reg.b.x;
    case 0b0011: return machine.reg.b.hl.l;
    case 0b1100: return machine.reg.sp;
    case 0b0100: return machine.reg.a.hl.h;
    case 0b1101: return machine.reg.bp;
    case 0b0101: return machine.reg.c.hl.h;
    case 0b1110: return machine.reg.si;
    case 0b0110: return machine.reg.d.hl.h;
    case 0b1111: return machine.reg.di;
    case 0b0111: return machine.reg.b.hl.h;
  }
  return 0;
}

unsigned short Inst::get_ea_value() {
  unsigned short ea;

  switch (rm & 0b111) {
    case 0b000: ea = machine.reg.b.x + machine.reg.si; break;
    case 0b001: ea = machine.reg.b.x + machine.reg.di; break;
    case 0b010: ea = machine.reg.bp + machine.reg.si; break;
    case 0b011: ea = machine.reg.bp + machine.reg.di; break;
    case 0b100: ea = machine.reg.si; break;
    case 0b101: ea = machine.reg.di; break;
    case 0b110: ea = machine.reg.bp; break;
    case 0b111: ea = machine.reg.b.x; break;
  }

  ea += disp;

  return ea;
}

int Inst::get_rm_value() {
  return get_rm_value(is_wide_data());
}

int Inst::get_rm_value(bool is_wide) {
  if (mod == 0b11) {
    return get_reg_value(true);
  }

  if (mod == 0b00 && rm == 0b110) {
    return is_wide ? machine.read_data_16(disp) : machine.read_data_8(disp);
  }

  unsigned short ea = get_ea_value();
  int value = is_wide ? machine.read_data_16(ea) : machine.read_data_8(ea);

  cout << "; ([" << hex << ea << "]=" << (is_wide ? "(w)" : "") << value << ")";

  return value;
}

int Inst::get_accum_value() {
  return is_wide_data() ? machine.reg.a.x : machine.reg.a.hl.l;
}

void Inst::put_reg_value(const unsigned int value, bool is_rm) {
  switch ((is_rm ? rm : reg) + (w << 3)) {
    case 0b1000: machine.reg.a.x = value; break;
    case 0b0000: machine.reg.a.hl.l = value; break;
    case 0b1001: machine.reg.c.x = value; break;
    case 0b0001: machine.reg.c.hl.l = value; break;
    case 0b1010: machine.reg.d.x = value; break;
    case 0b0010: machine.reg.d.hl.l = value; break;
    case 0b1011: machine.reg.b.x = value; break;
    case 0b0011: machine.reg.b.hl.l = value; break;
    case 0b1100: machine.reg.sp = value; break;
    case 0b0100: machine.reg.a.hl.h = value; break;
    case 0b1101: machine.reg.bp = value; break;
    case 0b0101: machine.reg.c.hl.h = value; break;
    case 0b1110: machine.reg.si = value; break;
    case 0b0110: machine.reg.d.hl.h = value; break;
    case 0b1111: machine.reg.di = value; break;
    case 0b0111: machine.reg.b.hl.h = value; break;
  }
}

void Inst::put_rm_value(const unsigned int value) {
  return put_rm_value(value, is_wide_data());
}

void Inst::put_rm_value(const unsigned int value, bool is_wide) {
  if (mod == 0b11) {
    put_reg_value(value, true);
    return;
  }

  char *data_seg = machine.get_data_seg();

  if (mod == 0b00 && rm == 0b110) {
    is_wide ? machine.write_data_16(disp, value) : machine.write_data_8(disp, value);
    return;
  }

  unsigned short ea = get_ea_value();

  is_wide ? machine.write_data_16(ea, value) : machine.write_data_8(ea, value);
}

void Inst::put_accum_value(const unsigned int value) {
  if (is_wide_data()) {
    machine.reg.a.x = value;
  } else {
    machine.reg.a.hl.l = value;
  }
}
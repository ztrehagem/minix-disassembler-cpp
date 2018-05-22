#include <sstream>
#include "inst.hpp"
#include "disassembler.hpp"

using namespace std;

void Inst::set_mod_sec() {
  has_mod_sec = true;
  mod = head[1] >> 6;
  reg = head[1] >> 3;
  rm = head[1];

  if (mod == 0b10 || (mod == 0b00 && rm == 0b110)) {
    disp = static_cast<signed short>(Disassembler::get_data_wide(&head[2]));
    disp_size = 2;
  } else if (mod == 0b01) {
    disp = static_cast<signed char>(Disassembler::get_data_narrow(&head[2]));
    disp_size = 1;
  }
}

void Inst::set_data() {
  const size_t offset = 1 + has_mod_sec + disp_size;
  has_data_sec = true;
  if (is_wide_data()) {
    data.wide = Disassembler::get_data_wide(&head[offset]);
    data_size = 2;
  } else {
    data.narrow = Disassembler::get_data_narrow(&head[offset]);
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
  return Disassembler::instruction_str(head, get_inst_len()) + op + ' ';
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
    return "[" + Disassembler::data_str_wide(disp) + "]";
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

string Inst::get_data_str(const bool as_natural) {
  if (is_wide_data()) {
    return Disassembler::data_str_wide(data.wide, as_natural);
  } else {
    return Disassembler::data_str_narrow(data.narrow, as_natural);
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

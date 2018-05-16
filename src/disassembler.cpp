#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include "disassembler.hpp"

using namespace std;

void Disassembler::disassemble() {
  ifs.read(reinterpret_cast<char *>(&header), sizeof(header));
  // cout << "a_text = " << header.a_text << endl;

  char text[header.a_text];
  ifs.read(text, sizeof(text));
  analyze_text(text, sizeof(text));

  cout << endl;
}

void Disassembler::analyze_text(const char text[], const size_t len) {
  size_t ti = 0;

  while (ti < len) {
    cout << line_number_str(ti);

    const char *head = &text[ti];
    const unsigned short top = head[0] & 0xff;
    const unsigned short dbl = (head[0] << 8 & 0xff00) + (head[1] & 0xff);

    if      ((top & 0b11111100)
                 == 0b10001000) ti += mov_1(head);
    else if ((top & 0b11110000)
                 == 0b10110000) ti += mov_3(head);
    else if ((dbl & 0b1111111100111000)
                 == 0b1111111100110000) ti += push_1(head);
    else if ((top & 0b11111000)
                 == 0b01010000) ti += push_2(head);
    else if ((top & 0b11111000)
                 == 0b01011000) ti += pop_2(head);
    else if ((top & 0b11111110)
                 == 0b11100100) ti += in_1(head);
    else if ((top & 0b11111110)
                 == 0b11101100) ti += in_2(head);
    else if ((top & 0b11111111)
                 == 0b10001101) ti += lea_1(head);
    else if ((top & 0b11111100)
                 == 0b00000000) ti += add_1(head);
    else if ((dbl & 0b1111110000111000)
                 == 0b1000000000000000) ti += add_2(head);
    else if ((dbl & 0b1111110000111000)
                 == 0b1000000000101000) ti += sub_2(head);
    else if ((top & 0b11111100)
                 == 0b00011000) ti += ssb_1(head);
    else if ((top & 0b11111000)
                 == 0b01001000) ti += dec_2(head);
    else if ((dbl & 0b1111111000111000)
                 == 0b1111011000011000) ti += neg_1(head);
    else if ((dbl & 0b1111110000111000)
                 == 0b1000000000111000) ti += cmp_2(head);
    else if ((dbl & 0b1111110000111000)
                 == 0b1101000000100000) ti += shl_1(head);
    else if ((top & 0b11111100)
                 == 0b00100000) ti += and_1(head);
    else if ((dbl & 0b1111111000111000)
                 == 0b1111011000000000) ti += test_2(head);
    else if ((top & 0b11111100)
                 == 0b00001000) ti += or_1(head);
    else if ((top & 0b11111110)
                 == 0b00001100) ti += or_3(head);
    else if ((top & 0b11111100)
                 == 0b00110000) ti += xor_1(head);
    else if ((top & 0b11111111)
                 == 0b11101000) ti += call_1(head, ti);
    else if ((dbl & 0b1111111100111000)
                 == 0b1111111100010000) ti += call_2(head);
    else if ((top & 0b11111111)
                 == 0b11101001) ti += jmp_1(head, ti);
    else if ((top & 0b11111111)
                 == 0b11101011) ti += jmp_2(head, ti);
    else if ((top & 0b11111111)
                 == 0b11000011) ti += ret_1(head);
    else if ((top & 0b11111111)
                 == 0b01110100) ti += je_1(head, ti);
    else if ((top & 0b11111111)
                 == 0b01111100) ti += jl_1(head, ti);
    else if ((top & 0b11111111)
                 == 0b01110101) ti += jne_1(head, ti);
    else if ((top & 0b11111111)
                 == 0b01111101) ti += jnl_1(head, ti);
    else if ((top & 0b11111111)
                 == 0b01110011) ti += jnb_1(head, ti);
    else if ((top & 0b11111111)
                 == 0b11001101) ti += int_1(head);
    else if ((top & 0b11111111)
                 == 0b11110100) ti += hlt_1(head);
    else {
      cout << instruction_str(head, 1);
      ti++;
    }

    cout << endl;
  }
}

size_t Disassembler::mov_1(const char *head) {
  Inst inst;
  inst.d = head[0] >> 1;
  inst.w = head[0];
  set_mod_sec(inst, head[1]);

  const size_t len = 2 + get_extended_len(inst);
  cout << instruction_str(head, len);
  cout << "mov ";

  if (inst.d) {
    cout << get_reg_name(inst) << ", " << get_rm_string(inst, &head[2]);
  } else {
    cout << get_rm_string(inst, &head[2]) << ", " << get_reg_name(inst);
  }

  return len;
}

size_t Disassembler::mov_3(const char *head) {
  Inst inst;
  inst.w = head[0] >> 3;
  inst.reg = head[0];
  set_data(inst, &head[1], inst.w);

  const size_t len = inst.w ? 3 : 2;
  cout << instruction_str(head, len);
  cout << "mov ";
  cout << get_reg_name(inst) << ", ";
  cout << inst_data_str(inst, inst.w);

  return len;
}

size_t Disassembler::push_1(const char *head) {
  Inst inst;
  set_mod_sec(inst, head[1]);

  const size_t len = 2 + get_extended_len(inst);
  cout << instruction_str(head, len);
  cout << "push ";
  cout << get_rm_string(inst, &head[2]);

  return len;
}

size_t Disassembler::push_2(const char *head) {
  Inst inst;
  inst.reg = head[0];
  inst.w = 1;

  const size_t len = 1;
  cout << instruction_str(head, len);
  cout << "push ";
  cout << get_reg_name(inst);

  return len;
}

size_t Disassembler::pop_2(const char *head) {
  Inst inst;
  inst.reg = head[0];
  inst.w = 1;

  const size_t len = 1;
  cout << instruction_str(head, len);
  cout << "pop ";
  cout << get_reg_name(inst);

  return len;
}

size_t Disassembler::in_1(const char *head) {
  Inst inst;
  inst.w = head[0];

  const size_t len = 2;
  cout << instruction_str(head, len);
  cout << "in ??, ??"; // FIXME

  return len;
}

size_t Disassembler::in_2(const char *head) {
  Inst inst;
  inst.w = head[0];

  const size_t len = 1;
  cout << instruction_str(head, len);
  cout << "in ??, ??"; // FIXME

  return len;
}

size_t Disassembler::lea_1(const char *head) {
  Inst inst;
  set_mod_sec(inst, head[1]);

  const size_t len = 2 + get_extended_len(inst);
  cout << instruction_str(head, len);
  cout << "lea ";
  cout << get_reg_name(inst) << ", " << get_rm_string(inst, &head[2]);

  return len;
}

size_t Disassembler::add_1(const char *head) {
  Inst inst;
  inst.d = head[0] >> 1;
  inst.w = head[0];
  set_mod_sec(inst, head[1]);

  const size_t len = 2 + get_extended_len(inst);
  cout << instruction_str(head, len);
  cout << "add ";

  if (inst.d) {
    cout << get_reg_name(inst) << ", " << get_rm_string(inst, &head[2]);
  } else {
    cout << get_rm_string(inst, &head[2]) << ", " << get_reg_name(inst);
  }

  return len;
}

size_t Disassembler::add_2(const char *head) {
  Inst inst;
  inst.s = head[0] >> 1;
  inst.w = head[0];
  set_mod_sec(inst, head[1]);
  const bool is_wide = inst.s == 0 && inst.w == 1;
  const size_t extended_len = get_extended_len(inst);
  set_data(inst, &head[2 + extended_len], is_wide);

  const size_t len = (is_wide ? 4 : 3) + extended_len;
  cout << instruction_str(head, len);
  cout << "add ";
  cout << get_rm_string(inst, &head[2]) << ", ";
  cout << inst_data_str(inst, is_wide, true);

  return len;
}

size_t Disassembler::sub_2(const char *head) {
  Inst inst;
  inst.s = head[0] >> 1;
  inst.w = head[0];
  set_mod_sec(inst, head[1]);
  const bool is_wide = inst.s == 0 && inst.w == 1;
  const size_t extended_len = get_extended_len(inst);
  set_data(inst, &head[2 + extended_len], is_wide);

  const size_t len = (is_wide ? 4 : 3) + extended_len;
  cout << instruction_str(head, len);
  cout << "sub ";
  cout << get_rm_string(inst, &head[2]) << ", ";
  cout << inst_data_str(inst, is_wide, true);

  return len;
}

size_t Disassembler::ssb_1(const char *head) {
  Inst inst;
  inst.d = head[0] >> 1;
  inst.w = head[0];
  set_mod_sec(inst, head[1]);

  const size_t len = 2 + get_extended_len(inst);
  cout << instruction_str(head, len);
  cout << "ssb ";
  if (inst.d) {
    cout << get_reg_name(inst) << ", " << get_rm_string(inst, &head[2]);
  } else {
    cout << get_rm_string(inst, &head[2]) << ", " << get_reg_name(inst);
  }

  return len;
}

size_t Disassembler::dec_2(const char *head) {
  Inst inst;
  inst.reg = head[0];
  inst.w = 1;

  const size_t len = 1;
  cout << instruction_str(head, len);
  cout << "dec ";
  cout << get_reg_name(inst);

  return len;
}

size_t Disassembler::neg_1(const char *head) {
  Inst inst;
  inst.w = head[0];
  set_mod_sec(inst, head[1]);

  const size_t len = 2 + get_extended_len(inst);
  cout << instruction_str(head, len);
  cout << "neg ";
  cout << get_rm_string(inst, &head[2]);

  return len;
}

size_t Disassembler::cmp_2(const char *head) {
  // Immediate with Register Memory
  Inst inst;
  inst.s = head[0] >> 1;
  inst.w = head[0];
  set_mod_sec(inst, head[1]);
  const bool is_wide = inst.s == 0 && inst.w == 1;
  const size_t extended_len = get_extended_len(inst);
  set_data(inst, &head[2 + extended_len], is_wide);

  const size_t len = (is_wide ? 4 : 3) + extended_len;
  cout << instruction_str(head, len);
  cout << "cmp ";
  cout << get_rm_string(inst, &head[2]) << ", ";
  cout << inst_data_str(inst, is_wide, true);

  return len;
}

size_t Disassembler::shl_1(const char *head) {
  Inst inst;
  inst.v = head[0] >> 1;
  inst.w = head[0];
  set_mod_sec(inst, head[1]);

  const size_t len = 2 + get_extended_len(inst);
  cout << instruction_str(head, len);
  cout << "shl ";
  cout << get_rm_string(inst, &head[2]) << ", " << (inst.v ? "[cl]" : "1");

  return len;
}

size_t Disassembler::and_1(const char *head) {
  Inst inst;
  inst.d = head[0] >> 1;
  inst.w = head[0];
  set_mod_sec(inst, head[1]);

  const size_t len = 2 + get_extended_len(inst);
  cout << instruction_str(head, len);
  cout << "and ";
  if (inst.d) {
    cout << get_reg_name(inst) << ", " << get_rm_string(inst, &head[2]);
  } else {
    cout << get_rm_string(inst, &head[2]) << ", " << get_reg_name(inst);
  }

  return len;
}

size_t Disassembler::test_2(const char *head) {
  Inst inst;
  inst.w = head[0];
  set_mod_sec(inst, head[1]);
  const size_t extended_len = get_extended_len(inst);
  set_data(inst, &head[2 + extended_len], inst.w);

  const size_t len = (inst.w ? 4 : 3) + extended_len;
  cout << instruction_str(head, len);
  cout << "test ";
  cout << get_rm_string(inst, &head[2]) << ", ";
  cout << inst_data_str(inst, inst.w, true);

  return len;
}

size_t Disassembler::or_1(const char *head) {
  Inst inst;
  inst.d = head[0] >> 1;
  inst.w = head[0];
  set_mod_sec(inst, head[1]);

  const size_t len = 2 + get_extended_len(inst);
  cout << instruction_str(head, len);
  cout << "or ";

  if (inst.d) {
    cout << get_reg_name(inst) << ", " << get_rm_string(inst, &head[2]);
  } else {
    cout << get_rm_string(inst, &head[2]) << ", " << get_reg_name(inst);
  }

  return len;
}

size_t Disassembler::or_3(const char *head) {
  Inst inst;
  inst.w = head[0];
  set_data(inst, &head[1], inst.w);

  const size_t len = inst.w ? 3 : 2;
  cout << instruction_str(head, len);
  cout << "or ";
  cout << inst_data_str(inst, inst.w);

  return len;
}

size_t Disassembler::xor_1(const char *head) {
  Inst inst;
  inst.d = head[0] >> 1;
  inst.w = head[0];
  set_mod_sec(inst, head[1]);

  const size_t len = 2 + get_extended_len(inst);
  cout << instruction_str(head, len);
  cout << "xor ";

  if (inst.d) {
    cout << get_reg_name(inst) << ", " << get_rm_string(inst, &head[2]);
  } else {
    cout << get_rm_string(inst, &head[2]) << ", " << get_reg_name(inst);
  }

  return len;
}

size_t Disassembler::call_1(const char *head, const size_t ti) {
  const size_t len = 3;
  cout << instruction_str(head, len);
  cout << "call ";
  cout << data_str_wide(ti + len + get_data_wide(&head[1]));
  return len;
}

size_t Disassembler::call_2(const char *head) {
  Inst inst;
  set_mod_sec(inst, head[1]);

  const size_t len = 2 + get_extended_len(inst);
  cout << instruction_str(head, len);
  cout << "call ";
  cout << get_rm_string(inst, &head[2]);
  return len;
}

size_t Disassembler::jmp_1(const char *head, const size_t ti) {
  const size_t len = 3;
  cout << instruction_str(head, len);
  cout << "jmp ";
  cout << data_str_wide(ti + len + static_cast<short>(get_data_wide(&head[1])));
  return len;
}

size_t Disassembler::jmp_2(const char *head, const size_t ti) {
  const size_t len = 2;
  cout << instruction_str(head, len);
  cout << "jmp short ";
  cout << data_str_wide(ti + len + static_cast<char>(get_data_narrow(&head[1])));
  return len;
}

size_t Disassembler::ret_1(const char *head) {
  const size_t len = 1;
  cout << instruction_str(head, len);
  cout << "ret";
  return len;
}

size_t Disassembler::je_1(const char *head, const size_t ti) {
  const size_t len = 2;
  cout << instruction_str(head, len);
  cout << "je ";
  cout << data_str_wide(ti + len + head[1]);
  return len;
}

size_t Disassembler::jl_1(const char *head, const size_t ti) {
  const size_t len = 2;
  cout << instruction_str(head, len);
  cout << "jl ";
  cout << data_str_wide(ti + len + head[1]);
  return len;
}

size_t Disassembler::jne_1(const char *head, const size_t ti) {
  const size_t len = 2;
  cout << instruction_str(head, len);
  cout << "jne ";
  cout << data_str_wide(ti + len + head[1]);
  return len;
}

size_t Disassembler::jnl_1(const char *head, const size_t ti) {
  const size_t len = 2;
  cout << instruction_str(head, len);
  cout << "jnl ";
  cout << data_str_wide(ti + len + head[1]);
  return len;
}

size_t Disassembler::jnb_1(const char *head, const size_t ti) {
  const size_t len = 2;
  cout << instruction_str(head, len);
  cout << "jnb ";
  cout << data_str_wide(ti + len + head[1]);
  return len;
}

size_t Disassembler::int_1(const char *head) {
  const size_t len = 2;
  cout << instruction_str(head, len);
  cout << "int ";
  cout << data_str_narrow(head[1]);
  return len;
}

size_t Disassembler::hlt_1(const char *head) {
  const size_t len = 1;
  cout << instruction_str(head, len);
  cout << "hlt";
  return len;
}

string Disassembler::get_reg_name(const Inst &inst, const bool is_rm) {
  switch (is_rm ? inst.rm : inst.reg) {
    case 0b000: return inst.w ? "ax" : "al";
    case 0b001: return inst.w ? "cx" : "cl";
    case 0b010: return inst.w ? "dx" : "dl";
    case 0b011: return inst.w ? "bx" : "bl";
    case 0b100: return inst.w ? "sp" : "ah";
    case 0b101: return inst.w ? "bp" : "ch";
    case 0b110: return inst.w ? "si" : "dh";
    case 0b111: return inst.w ? "di" : "bh";
    default: return "";
  }
}

string Disassembler::get_rm_string(const Inst &inst, const char *extended) {
  int disp = 0;

  switch (inst.mod & 0b11) {
    case 0b11: return get_reg_name(inst, true);
    case 0b00: break;
    case 0b10: disp = static_cast<short>(get_data_wide(extended)); break;
    case 0b01: disp = static_cast<char>(get_data_narrow(extended)); break;
  }

  string ea;

  switch (inst.rm & 0b111) {
    case 0b000: ea = "bx+si"; break;
    case 0b001: ea = "bx+di"; break;
    case 0b010: ea = "bp+si"; break;
    case 0b011: ea = "bp+di"; break;
    case 0b100: ea = "si"; break;
    case 0b101: ea = "di"; break;
    case 0b110: ea = "bp"; break;
    case 0b111: ea = "bx"; break;
  }

  ostringstream oss;

  if (inst.mod == 0b00 && inst.rm == 0b110) {
    oss << setfill('0') << setw(4) << hex << static_cast<short>(get_data_wide(extended));
    ea = oss.str();
  } else if (disp > 0) {
    oss << hex << disp;
    ea += "+" + oss.str();
  } else if (disp < 0) {
    oss << hex << -disp;
    ea += "-" + oss.str();
  }

  return "[" + ea + "]";
}

void Disassembler::set_mod_sec(Inst &inst, const char byte) {
  inst.mod = byte >> 6;
  inst.reg = byte >> 3;
  inst.rm = byte;
}

void Disassembler::set_data(Inst &inst, const char *head, const bool is_wide) {
  if (is_wide) {
    inst.data.wide = get_data_wide(head);
  } else {
    inst.data.narrow = get_data_narrow(head);
  }
}

unsigned short Disassembler::get_data_wide(const char *head) {
  return (head[0] & 0xff) + ((head[1] & 0xff) << 8);
}

unsigned char Disassembler::get_data_narrow(const char *head) {
  return head[0] & 0xff;
}

size_t Disassembler::get_extended_len(const Inst &inst) {
  switch (inst.mod) {
    case 0b10: return 2;
    case 0b01: return 1;
    case 0b00: return inst.rm == 0b110 ? 2 : 0;
    default: return 0;
  }
}

string Disassembler::inst_data_str(const Inst &inst, const bool is_wide, const bool as_natural) {
  if (is_wide) {
    return data_str_wide(inst.data.wide, as_natural);
  } else {
    return data_str_narrow(inst.data.narrow, as_natural);
  }
}

string Disassembler::data_str_wide(const unsigned short data, const bool as_natural) {
  return hex_str(data & 0xffff, as_natural ? 0 : 4);
}

string Disassembler::data_str_narrow(const unsigned char data, const bool as_natural) {
  return hex_str(data & 0xff, as_natural ? 0 : 2);
}

string Disassembler::line_number_str(const size_t n) {
  return hex_str(n & 0xffff, 4) + ": ";
}

string Disassembler::instruction_str(const char *head, const size_t len) {
  ostringstream ss;
  size_t i = 0;
  for (; i < len; i++) ss << hex_str(head[i] & 0xff, 2);
  for (; i < 7; i++) ss << "  ";
  return ss.str();
}

string Disassembler::hex_str(const unsigned int value, const size_t w) {
  ostringstream ss;
  if (w > 0) {
    ss << setfill('0') << setw(w) << hex << value;
  } else {
    ss << hex << value;
  }
  return ss.str();
}

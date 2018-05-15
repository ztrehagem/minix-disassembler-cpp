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
  while (ti < len) {
    print_line_number(ti);

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
    else if ((top & 0b11111000)
                 == 0b01001000) ti += dec_2(head);
    else if ((dbl & 0b1111110000111000)
                 == 0b1000000000111000) ti += cmp_2(head);
    else if ((dbl & 0b1111110000111000)
                 == 0b1101000000100000) ti += shl_1(head);
    else if ((top & 0b11111100)
                 == 0b00100000) ti += and_1(head);
    else if ((dbl & 0b1111111000111000)
                 == 0b1111011000000000) ti += test_2(head);
    else if ((top & 0b11111110)
                 == 0b00001100) ti += or_3(head);
    else if ((top & 0b11111100)
                 == 0b00110000) ti += xor_1(head);
    else if ((top & 0b11111111)
                 == 0b11101000) ti += call_1(head);
    else if ((dbl & 0b1111111100111000)
                 == 0b1111111100010000) ti += call_2(head);
    else if ((top & 0b11111111)
                 == 0b11101001) ti += jmp_1(head);
    else if ((top & 0b11111111)
                 == 0b11101011) ti += jmp_2(head);
    else if ((top & 0b11111111)
                 == 0b01110100) ti += je_1(head);
    else if ((top & 0b11111111)
                 == 0b01111100) ti += jl_1(head);
    else if ((top & 0b11111111)
                 == 0b01110101) ti += jne_1(head);
    else if ((top & 0b11111111)
                 == 0b01110011) ti += jnb_1(head);
    else if ((top & 0b11111111)
                 == 0b11001101) ti += int_1(head);
    else if ((top & 0b11111111)
                 == 0b11110100) ti += hlt_1(head);
    else {
      print_byte(head[0]);
      ti++;
    }

    cout << endl;
  }
}

size_t Disassembler::mov_1(const char *head) {
  Inst inst;
  inst.d = head[0] >> 1;
  inst.w = head[0];
  inst.mod = head[1] >> 6;
  inst.reg = head[1] >> 3;
  inst.rm = head[1];

  const size_t len = 2 + get_extended_len(inst);
  print_bytes(head, len);
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
  print_bytes(head, len);
  cout << "mov ";
  cout << get_reg_name(inst) << ", ";
  print_data(inst, inst.w);

  return len;
}

size_t Disassembler::push_1(const char *head) {
  Inst inst;
  inst.mod = head[1] >> 6;
  inst.rm = head[1];

  const size_t len = 2 + get_extended_len(inst);
  print_bytes(head, len);
  cout << "push ";
  cout << get_rm_string(inst, &head[2]);

  return len;
}

size_t Disassembler::push_2(const char *head) {
  Inst inst;
  inst.reg = head[0];
  inst.w = 1;

  const size_t len = 1;
  print_bytes(head, len);
  cout << "push ";
  cout << get_reg_name(inst);

  return len;
}

size_t Disassembler::pop_2(const char *head) {
  Inst inst;
  inst.reg = head[0];
  inst.w = 1;

  const size_t len = 1;
  print_bytes(head, len);
  cout << "pop ";
  cout << get_reg_name(inst);

  return len;
}

size_t Disassembler::in_1(const char *head) {
  Inst inst;
  inst.w = head[0];

  const size_t len = 2;
  print_bytes(head, len);
  cout << "in ??, ??"; // FIXME

  return len;
}

size_t Disassembler::in_2(const char *head) {
  Inst inst;
  inst.w = head[0];

  const size_t len = 1;
  print_bytes(head, len);
  cout << "in ??, ??"; // FIXME

  return len;
}

size_t Disassembler::lea_1(const char *head) {
  Inst inst;
  inst.mod = head[1] >> 6;
  inst.reg = head[1] >> 3;
  inst.rm = head[1];

  const size_t len = inst.mod == 0b01 ? 3 : 2;
  print_bytes(head, len);
  cout << "lea ";
  cout << get_reg_name(inst) << ", " << get_rm_string(inst, &head[2]);

  return len;
}

size_t Disassembler::add_1(const char *head) {
  Inst inst;
  inst.d = head[0] >> 1;
  inst.w = head[0];
  inst.mod = head[1] >> 6;
  inst.reg = head[1] >> 3;
  inst.rm = head[1];

  const size_t len = 2 + get_extended_len(inst);
  print_bytes(head, len);
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
  inst.mod = head[1] >> 6;
  inst.rm = head[1];
  const bool is_wide = inst.s == 0 && inst.w == 1;
  set_data(inst, &head[2], is_wide);

  const size_t len = is_wide ? 4 : 3;
  print_bytes(head, len);
  cout << "add ";
  cout << get_rm_string(inst) << ", ";
  print_data(inst, is_wide, true);

  return len;
}

size_t Disassembler::dec_2(const char *head) {
  Inst inst;
  inst.reg = head[0];
  inst.w = 1;

  const size_t len = 1;
  print_bytes(head, len);
  cout << "dec ";
  cout << get_reg_name(inst);

  return len;
}

size_t Disassembler::cmp_2(const char *head) {
  // Immediate with Register Memory
  Inst inst;
  inst.s = head[0] >> 1;
  inst.w = head[0];
  inst.mod = head[1] >> 6;
  inst.rm = head[1];
  const bool is_wide = inst.s == 0 && inst.w == 1;
  const size_t extended_len = get_extended_len(inst);
  set_data(inst, &head[2 + extended_len], is_wide);

  const size_t len = (is_wide ? 4 : 3) + get_extended_len(inst);
  print_bytes(head, len);
  cout << "cmp ";
  cout << get_rm_string(inst, &head[2]) << ", ";
  print_data(inst, is_wide, true);

  return len;
}

size_t Disassembler::shl_1(const char *head) {
  Inst inst;
  inst.v = head[0] >> 1;
  inst.w = head[0];
  inst.mod = head[1] >> 6;
  inst.rm = head[1];

  const size_t len = 2;
  print_bytes(head, len);
  cout << "shl ";
  cout << get_rm_string(inst) << ", " << (inst.v ? "[cl]" : "1");

  return len;
}

size_t Disassembler::and_1(const char *head) {
  Inst inst;
  inst.d = head[0] >> 1;
  inst.w = head[0];
  inst.mod = head[1] >> 6;
  inst.reg = head[1] >> 3;
  inst.rm = head[1];

  const size_t len = 2 + get_extended_len(inst);
  print_bytes(head, len);
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
  inst.mod = head[1] >> 6;
  inst.rm = head[1];
  set_data(inst, &head[2], inst.w);

  const size_t len = inst.w ? 4 : 3;
  print_bytes(head, len);
  cout << "test ";
  cout << get_rm_string(inst, &head[2]) << ", ";
  print_data(inst, inst.w, true);

  return len;
}

size_t Disassembler::or_3(const char *head) {
  Inst inst;
  inst.w = head[0];
  set_data(inst, &head[1], inst.w);

  const size_t len = inst.w ? 3 : 2;
  print_bytes(head, len);
  cout << "or ";
  print_data(inst, inst.w);

  return len;
}

size_t Disassembler::xor_1(const char *head) {
  Inst inst;
  inst.d = head[0] >> 1;
  inst.w = head[0];
  inst.mod = head[1] >> 6;
  inst.reg = head[1] >> 3;
  inst.rm = head[1];

  const size_t len = 2;
  print_bytes(head, len);
  cout << "xor ";

  if (inst.d) {
    cout << get_reg_name(inst) << ", " << get_rm_string(inst);
  } else {
    cout << get_rm_string(inst) << ", " << get_reg_name(inst);
  }

  return len;
}

size_t Disassembler::call_1(const char *head) {
  const size_t len = 3;
  print_bytes(head, len);
  cout << "call ";
  print_data_wide(ti + len + get_data_wide(&head[1]));
  return len;
}

size_t Disassembler::call_2(const char *head) {
  Inst inst;
  inst.mod = head[1] >> 6;
  inst.rm = head[1];

  const size_t len = 2;
  print_bytes(head, len);
  cout << "call ";
  cout << get_rm_string(inst);
  return len;
}

size_t Disassembler::jmp_1(const char *head) {
  const size_t len = 3;
  print_bytes(head, len);
  cout << "jmp ";
  print_data_wide(ti + len + static_cast<short>(get_data_wide(&head[1])));
  return len;
}

size_t Disassembler::jmp_2(const char *head) {
  const size_t len = 2;
  print_bytes(head, len);
  cout << "jmp short ";
  print_data_wide(ti + len + static_cast<char>(get_data_narrow(&head[1])));
  return len;
}

size_t Disassembler::je_1(const char *head) {
  const size_t len = 2;
  print_bytes(head, len);
  cout << "je ";
  print_data_wide(ti + len + head[1]);
  return len;
}

size_t Disassembler::jl_1(const char *head) {
  const size_t len = 2;
  print_bytes(head, len);
  cout << "jl ";
  print_data_wide(ti + len + head[1]);
  return len;
}

size_t Disassembler::jne_1(const char *head) {
  const size_t len = 2;
  print_bytes(head, len);
  cout << "jne ";
  print_data_wide(ti + len + head[1]);
  return len;
}

size_t Disassembler::jnb_1(const char *head) {
  const size_t len = 2;
  print_bytes(head, len);
  cout << "jnb ";
  print_data_wide(ti + len + head[1]);
  return len;
}

size_t Disassembler::int_1(const char *head) {
  const size_t len = 2;
  print_bytes(head, len);
  cout << "int ";
  print_data_narrow(head[1]);
  return len;
}

size_t Disassembler::hlt_1(const char *head) {
  const size_t len = 1;
  print_bytes(head, len);
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

void Disassembler::print_data(const Inst &inst, const bool is_wide, const bool as_natural) {
  if (is_wide) {
    print_data_wide(inst.data.wide, as_natural);
  } else {
    print_data_narrow(inst.data.narrow, as_natural);
  }
}

void Disassembler::print_data_wide(const unsigned short data, const bool as_natural) {
  unsigned int v = data & 0xffff;
  if (as_natural) {
    cout << hex << v;
  } else {
    cout << setfill('0') << setw(4) << hex << v;
  }
}

void Disassembler::print_data_narrow(const unsigned char data, const bool as_natural) {
  unsigned int v = data & 0xff;
  if (as_natural) {
    cout << hex << v;
  } else {
    cout << setfill('0') << setw(2) << hex << v;
  }
}

void Disassembler::print_line_number(const size_t n) {
  cout << setfill('0') << setw(4) << hex << n << ": ";
}

void Disassembler::print_bytes(const char *head, const size_t len) {
  size_t i = 0;
  for (; i < len; i++) {
    print_byte(head[i]);
  }
  for (; i < 7; i++) {
    cout << "  ";
  }
}

void Disassembler::print_byte(const char c) {
  cout << setfill('0') << setw(2) << hex << static_cast<int>(0xff & c);
}

#include <iostream>
#include <iomanip>
#include <fstream>
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
    else if ((top & 0b11111000)
                 == 0b01010000) ti += push_2(head);
    else if ((top & 0b11111111)
                 == 0b10001101) ti += lea_1(head);
    else if ((top & 0b11111100)
                 == 0b00000000) ti += add_1(head);
    else if ((dbl & 0b1111110000111000)
                 == 0b1000000000111000) ti += cmp_2(head);
    else if ((dbl & 0b1111111000111000)
                 == 0b1111011000000000) ti += test_2(head);
    else if ((top & 0b11111100)
                 == 0b00110000) ti += xor_1(head);
    else if ((top & 0b11111111)
                 == 0b11101000) ti += call_1(head);
    else if ((top & 0b11111111)
                 == 0b01110101) ti += jne_1(head);
    else if ((top & 0b11111111)
                 == 0b01110011) ti += jnb_1(head);
    else if ((top & 0b11111111)
                 == 0b11001101) ti += int_1(head);
    else {
      print_byte(head[0]);
      cout << "\t is not implemented";
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

  const size_t len = 2;
  print_bytes(head, len);
  cout << "\t mov ";

  if (inst.d) {
    cout << get_reg_name(inst) << ", " << get_rm_string(inst);
  } else {
    cout << get_rm_string(inst) << ", " << get_reg_name(inst);
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
  cout << "\t mov ";
  cout << get_reg_name(inst) << ", ";
  print_data(inst, inst.w);

  return len;
}

size_t Disassembler::push_2(const char *head) {
  Inst inst;
  inst.reg = head[0];
  inst.w = 1;

  const size_t len = 1;
  print_bytes(head, len);
  cout << "\t push ";
  cout << get_reg_name(inst);

  return len;
}

size_t Disassembler::lea_1(const char *head) {
  Inst inst;
  inst.mod = head[1] >> 6;
  inst.reg = head[1] >> 3;
  inst.rm = head[1];

  const size_t len = inst.mod == 0b01 ? 3 : 2;
  print_bytes(head, len);
  cout << "\t lea ";
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

  const size_t len = 2;
  print_bytes(head, len);
  cout << "\t add ";

  if (inst.d) {
    cout << get_reg_name(inst) << ", " << get_rm_string(inst);
  } else {
    cout << get_rm_string(inst) << ", " << get_reg_name(inst);
  }

  return len;
}

size_t Disassembler::cmp_2(const char *head) {
  // Immediate with Register Memory
  Inst inst;
  inst.s = head[0] >> 1;
  inst.w = head[0];
  inst.mod = head[1] >> 6;
  inst.rm = head[1];
  set_data(inst, &head[2], inst.w);

  const size_t len = inst.w ? 4 : 3;
  print_bytes(head, len);
  cout << "\t cmp ";
  cout << get_rm_string(inst) << ", ";
  print_data(inst, inst.w);

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
  cout << "\t test ";
  cout << get_reg_name(inst) << ", ";
  print_data(inst, inst.w, true);

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
  cout << "\t xor ";

  if (inst.d) {
    cout << get_reg_name(inst) << ", " << get_rm_string(inst);
  } else {
    cout << get_rm_string(inst) << ", " << get_reg_name(inst);
  }

  return len;
}

size_t Disassembler::call_1(const char *head) {
  Inst inst;
  set_data(inst, &head[1], true);

  const size_t len = 3;
  print_bytes(head, len);
  cout << "\t call ";
  print_data_wide(ti + len + inst.data.wide);
  return len;
}

size_t Disassembler::jne_1(const char *head) {
  const size_t len = 2;
  print_bytes(head, len);
  cout << "\t jne ";
  print_data_wide(ti + len + head[1]);
  return len;
}

size_t Disassembler::jnb_1(const char *head) {
  const size_t len = 2;
  print_bytes(head, len);
  cout << "\t jnb ";
  print_data_wide(ti + len + head[1]);
  return len;
}

size_t Disassembler::int_1(const char *head) {
  const size_t len = 2;
  print_bytes(head, len);
  cout << "\t int ";
  print_data_narrow(head[1]);
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
  unsigned char disp = 0;

  switch (inst.mod & 0b11) {
    case 0b11: return get_reg_name(inst, true);
    case 0b00: disp = 0; break;
    case 0b10: disp = extended[0]; break;
    case 0b01: disp = extended[0]; break;
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

  if (disp != 0) {
    ea += "+" + to_string(disp);
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
    cout << v;
  } else {
    cout << setfill('0') << setw(4) << hex << v;
  }
}

void Disassembler::print_data_narrow(const unsigned char data, const bool as_natural) {
  unsigned int v = data & 0xff;
  if (as_natural) {
    cout << v;
  } else {
    cout << setfill('0') << setw(2) << hex << v;
  }
}

void Disassembler::print_line_number(const size_t n) {
  cout << setfill('0') << setw(4) << hex << n << ": ";
}

void Disassembler::print_bytes(const char *head, const size_t len) {
  for (size_t i = 0; i < len; i++) {
    print_byte(head[i]);
  }
}

void Disassembler::print_byte(const char c) {
  cout << setfill('0') << setw(2) << hex << static_cast<int>(0xff & c);
}

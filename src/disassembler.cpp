#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include "disassembler.hpp"
#include "inst.hpp"

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
  size_t pc = 0;

  while (pc < len) {
    cout << line_number_str(pc);

    const char *head = &text[pc];
    const unsigned short top = head[0] & 0xff;
    const unsigned short dbl = (head[0] << 8 & 0xff00) + (head[1] & 0xff);

    if      ((top & 0b11111100)
                 == 0b10001000) pc += mov_1(head);
    else if ((top & 0b11110000)
                 == 0b10110000) pc += mov_3(head);
    else if ((dbl & 0b1111111100111000)
                 == 0b1111111100110000) pc += push_1(head);
    else if ((top & 0b11111000)
                 == 0b01010000) pc += push_2(head);
    else if ((top & 0b11111000)
                 == 0b01011000) pc += pop_2(head);
    else if ((top & 0b11111110)
                 == 0b11100100) pc += in_1(head);
    else if ((top & 0b11111110)
                 == 0b11101100) pc += in_2(head);
    else if ((top & 0b11111111)
                 == 0b10001101) pc += lea_1(head);
    else if ((top & 0b11111100)
                 == 0b00000000) pc += add_1(head);
    else if ((dbl & 0b1111110000111000)
                 == 0b1000000000000000) pc += add_2(head);
    else if ((dbl & 0b1111110000111000)
                 == 0b1000000000101000) pc += sub_2(head);
    else if ((top & 0b11111100)
                 == 0b00011000) pc += ssb_1(head);
    else if ((top & 0b11111000)
                 == 0b01001000) pc += dec_2(head);
    else if ((dbl & 0b1111111000111000)
                 == 0b1111011000011000) pc += neg_1(head);
    else if ((dbl & 0b1111110000111000)
                 == 0b1000000000111000) pc += cmp_2(head);
    else if ((dbl & 0b1111110000111000)
                 == 0b1101000000100000) pc += shl_1(head);
    else if ((top & 0b11111100)
                 == 0b00100000) pc += and_1(head);
    else if ((dbl & 0b1111111000111000)
                 == 0b1111011000000000) pc += test_2(head);
    else if ((top & 0b11111100)
                 == 0b00001000) pc += or_1(head);
    else if ((top & 0b11111110)
                 == 0b00001100) pc += or_3(head);
    else if ((top & 0b11111100)
                 == 0b00110000) pc += xor_1(head);
    else if ((top & 0b11111111)
                 == 0b11101000) pc += call_1(head, pc);
    else if ((dbl & 0b1111111100111000)
                 == 0b1111111100010000) pc += call_2(head);
    else if ((top & 0b11111111)
                 == 0b11101001) pc += jmp_1(head, pc);
    else if ((top & 0b11111111)
                 == 0b11101011) pc += jmp_2(head, pc);
    else if ((top & 0b11111111)
                 == 0b11000011) pc += ret_1(head);
    else if ((top & 0b11111111)
                 == 0b01110100) pc += je_1(head, pc);
    else if ((top & 0b11111111)
                 == 0b01111100) pc += jl_1(head, pc);
    else if ((top & 0b11111111)
                 == 0b01110101) pc += jne_1(head, pc);
    else if ((top & 0b11111111)
                 == 0b01111101) pc += jnl_1(head, pc);
    else if ((top & 0b11111111)
                 == 0b01110011) pc += jnb_1(head, pc);
    else if ((top & 0b11111111)
                 == 0b11001101) pc += int_1(head);
    else if ((top & 0b11111111)
                 == 0b11110100) pc += hlt_1(head);
    else {
      cout << instruction_str(head, 1);
      pc++;
    }

    cout << endl;
  }
}

size_t Disassembler::mov_1(const char *head) {
  Inst inst;
  inst.d = head[0] >> 1;
  inst.w = head[0];
  inst.set_mod_sec(head);

  const size_t len = inst.get_inst_len();
  cout << instruction_str(head, len);
  cout << "mov ";
  cout << inst.get_dist_str();

  return len;
}

size_t Disassembler::mov_3(const char *head) {
  Inst inst;
  inst.w = head[0] >> 3;
  inst.reg = head[0];
  inst.set_data(head);

  const size_t len = inst.get_inst_len();
  cout << instruction_str(head, len);
  cout << "mov ";
  cout << inst.get_reg_name() << ", " << inst.get_data_str();

  return len;
}

size_t Disassembler::push_1(const char *head) {
  Inst inst;
  inst.set_mod_sec(head);

  const size_t len = inst.get_inst_len();
  cout << instruction_str(head, len);
  cout << "push ";
  cout << inst.get_rm_str();

  return len;
}

size_t Disassembler::push_2(const char *head) {
  Inst inst;
  inst.reg = head[0];

  const size_t len = inst.get_inst_len();
  cout << instruction_str(head, len);
  cout << "push ";
  cout << inst.get_reg_name();

  return len;
}

size_t Disassembler::pop_2(const char *head) {
  Inst inst;
  inst.reg = head[0];

  const size_t len = inst.get_inst_len();
  cout << instruction_str(head, len);
  cout << "pop ";
  cout << inst.get_reg_name();

  return len;
}

size_t Disassembler::in_1(const char *head) {
  const size_t len = 2;
  cout << instruction_str(head, len);
  cout << "in ??, ??"; // FIXME

  return len;
}

size_t Disassembler::in_2(const char *head) {
  const size_t len = 1;
  cout << instruction_str(head, len);
  cout << "in ??, ??"; // FIXME

  return len;
}

size_t Disassembler::lea_1(const char *head) {
  Inst inst;
  inst.set_mod_sec(head);

  const size_t len = inst.get_inst_len();
  cout << instruction_str(head, len);
  cout << "lea ";
  cout << inst.get_reg_name() << ", " << inst.get_rm_str();

  return len;
}

size_t Disassembler::add_1(const char *head) {
  Inst inst;
  inst.d = head[0] >> 1;
  inst.w = head[0];
  inst.set_mod_sec(head);

  const size_t len = inst.get_inst_len();
  cout << instruction_str(head, len);
  cout << "add ";
  cout << inst.get_dist_str();

  return len;
}

size_t Disassembler::add_2(const char *head) {
  Inst inst;
  inst.s = head[0] >> 1;
  inst.w = head[0];
  inst.set_mod_sec(head);
  inst.set_data(head);

  const size_t len = inst.get_inst_len();
  cout << instruction_str(head, len);
  cout << "add ";
  cout << inst.get_rm_str() << ", " << inst.get_data_str(true);

  return len;
}

size_t Disassembler::sub_2(const char *head) {
  Inst inst;
  inst.s = head[0] >> 1;
  inst.w = head[0];
  inst.set_mod_sec(head);
  inst.set_data(head);

  const size_t len = inst.get_inst_len();
  cout << instruction_str(head, len);
  cout << "sub ";
  cout << inst.get_rm_str() << ", " << inst.get_data_str(true);

  return len;
}

size_t Disassembler::ssb_1(const char *head) {
  Inst inst;
  inst.d = head[0] >> 1;
  inst.w = head[0];
  inst.set_mod_sec(head);

  const size_t len = inst.get_inst_len();
  cout << instruction_str(head, len);
  cout << "ssb ";
  cout << inst.get_dist_str();
  return len;
}

size_t Disassembler::dec_2(const char *head) {
  Inst inst;
  inst.reg = head[0];

  const size_t len = inst.get_inst_len();
  cout << instruction_str(head, len);
  cout << "dec ";
  cout << inst.get_reg_name();

  return len;
}

size_t Disassembler::neg_1(const char *head) {
  Inst inst;
  inst.w = head[0];
  inst.set_mod_sec(head);

  const size_t len = inst.get_inst_len();
  cout << instruction_str(head, len);
  cout << "neg ";
  cout << inst.get_rm_str();

  return len;
}

size_t Disassembler::cmp_2(const char *head) {
  // Immediate with Register Memory
  Inst inst;
  inst.s = head[0] >> 1;
  inst.w = head[0];
  inst.set_mod_sec(head);
  inst.set_data(head);

  const size_t len = inst.get_inst_len();
  cout << instruction_str(head, len);
  cout << "cmp ";
  cout << inst.get_rm_str() << ", " << inst.get_data_str(true);

  return len;
}

size_t Disassembler::shl_1(const char *head) {
  Inst inst;
  inst.v = head[0] >> 1;
  inst.w = head[0];
  inst.set_mod_sec(head);

  const size_t len = inst.get_inst_len();
  cout << instruction_str(head, len);
  cout << "shl ";
  cout << inst.get_rm_str() << ", " << (inst.v ? "[cl]" : "1");

  return len;
}

size_t Disassembler::and_1(const char *head) {
  Inst inst;
  inst.d = head[0] >> 1;
  inst.w = head[0];
  inst.set_mod_sec(head);

  const size_t len = inst.get_inst_len();
  cout << instruction_str(head, len);
  cout << "and ";
  cout << inst.get_dist_str();

  return len;
}

size_t Disassembler::test_2(const char *head) {
  Inst inst;
  inst.w = head[0];
  inst.set_mod_sec(head);
  inst.set_data(head);

  const size_t len = inst.get_inst_len();
  cout << instruction_str(head, len);
  cout << "test ";
  cout << inst.get_rm_str() << ", " << inst.get_data_str(true);

  return len;
}

size_t Disassembler::or_1(const char *head) {
  Inst inst;
  inst.d = head[0] >> 1;
  inst.w = head[0];
  inst.set_mod_sec(head);

  const size_t len = inst.get_inst_len();
  cout << instruction_str(head, len);
  cout << "or ";
  cout << inst.get_dist_str();

  return len;
}

size_t Disassembler::or_3(const char *head) {
  Inst inst;
  inst.w = head[0];
  inst.set_data(head);

  const size_t len = inst.get_inst_len();
  cout << instruction_str(head, len);
  cout << "or ";
  cout << inst.get_data_str();

  return len;
}

size_t Disassembler::xor_1(const char *head) {
  Inst inst;
  inst.d = head[0] >> 1;
  inst.w = head[0];
  inst.set_mod_sec(head);

  const size_t len = inst.get_inst_len();
  cout << instruction_str(head, len);
  cout << "xor ";
  cout << inst.get_dist_str();

  return len;
}

size_t Disassembler::call_1(const char *head, const size_t pc) {
  const short disp = get_data_wide(&head[1]);
  const size_t len = 3;
  cout << instruction_str(head, len);
  cout << "call ";
  cout << data_str_wide(pc + len + disp);

  return len;
}

size_t Disassembler::call_2(const char *head) {
  Inst inst;
  inst.set_mod_sec(head);

  const size_t len = inst.get_inst_len();
  cout << instruction_str(head, len);
  cout << "call ";
  cout << inst.get_rm_str();

  return len;
}

size_t Disassembler::jmp_1(const char *head, const size_t pc) {
  const short disp = get_data_wide(&head[1]);
  const size_t len = 3;
  cout << instruction_str(head, len);
  cout << "jmp ";
  cout << data_str_wide(pc + len + disp);
  return len;
}

size_t Disassembler::jmp_2(const char *head, const size_t pc) {
  const char disp = get_data_narrow(&head[1]);
  const size_t len = 2;
  cout << instruction_str(head, len);
  cout << "jmp short ";
  cout << data_str_wide(pc + len + disp);
  return len;
}

size_t Disassembler::ret_1(const char *head) {
  const size_t len = 1;
  cout << instruction_str(head, len);
  cout << "ret";
  return len;
}

size_t Disassembler::je_1(const char *head, const size_t pc) {
  const char disp = get_data_narrow(&head[1]);
  const size_t len = 2;
  cout << instruction_str(head, len);
  cout << "je ";
  cout << data_str_wide(pc + len + disp);
  return len;
}

size_t Disassembler::jl_1(const char *head, const size_t pc) {
  const char disp = get_data_narrow(&head[1]);
  const size_t len = 2;
  cout << instruction_str(head, len);
  cout << "jl ";
  cout << data_str_wide(pc + len + disp);
  return len;
}

size_t Disassembler::jne_1(const char *head, const size_t pc) {
  const char disp = get_data_narrow(&head[1]);
  const size_t len = 2;
  cout << instruction_str(head, len);
  cout << "jne ";
  cout << data_str_wide(pc + len + disp);
  return len;
}

size_t Disassembler::jnl_1(const char *head, const size_t pc) {
  const char disp = get_data_narrow(&head[1]);
  const size_t len = 2;
  cout << instruction_str(head, len);
  cout << "jnl ";
  cout << data_str_wide(pc + len + disp);
  return len;
}

size_t Disassembler::jnb_1(const char *head, const size_t pc) {
  const char disp = get_data_narrow(&head[1]);
  const size_t len = 2;
  cout << instruction_str(head, len);
  cout << "jnb ";
  cout << data_str_wide(pc + len + disp);
  return len;
}

size_t Disassembler::int_1(const char *head) {
  const unsigned char type = get_data_narrow(&head[1]);
  const size_t len = 2;
  cout << instruction_str(head, len);
  cout << "int ";
  cout << data_str_narrow(type);
  return len;
}

size_t Disassembler::hlt_1(const char *head) {
  const size_t len = 1;
  cout << instruction_str(head, len);
  cout << "hlt";
  return len;
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

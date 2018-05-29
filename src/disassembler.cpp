#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include "disassembler.hpp"
#include "inst.hpp"
#include "consts.hpp"

using namespace std;

#define SIGNED true
#define UNSIGNED false

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
                 == 0b10001000) pc += proc_rm_and_reg_to_either(head, "mov", true);
    else if ((dbl & 0b1111111000111000)
                 == 0b1100011000000000) pc += proc_imm_to_rm(head, "mov", SIGNED);
    else if ((top & 0b11110000)
                 == 0b10110000) pc += proc_imm_to_reg(head, "mov");
    else if ((dbl & 0b1111111100111000)
                 == 0b1111111100110000) pc += proc_rm(head, "push");
    else if ((top & 0b11111000)
                 == 0b01010000) pc += proc_reg(head, "push");
    else if ((dbl & 0b1111111100111000)
                 == 0b1000111100000000) pc += proc_rm(head, "pop");
    else if ((top & 0b11111000)
                 == 0b01011000) pc += proc_reg(head, "pop");
    else if ((top & 0b11111110)
                 == 0b10000110) pc += proc_rm_and_reg_to_either(head, "xchg");
    else if ((top & 0b11111000)
                 == 0b10010000) pc += proc_reg_with_accum(head, "xchg");
    else if ((top & 0b11111110)
                 == 0b11100100) pc += inst_in_1(head);
    else if ((top & 0b11111110)
                 == 0b11101100) pc += inst_in_2(head);
    else if ((top & 0b11111111)
                 == 0b10001101) pc += inst_lea(head);
    else if ((top & 0b11111100)
                 == 0b00000000) pc += proc_rm_and_reg_to_either(head, "add", true);
    else if ((dbl & 0b1111110000111000)
                 == 0b1000000000000000) pc += proc_imm_to_rm(head, "add", SIGNED, true);
    else if ((top & 0b11111110)
                 == 0b00000100) pc += proc_imm_to_accum(head, "add");
    else if ((top & 0b11111100)
                 == 0b00010000) pc += proc_rm_and_reg_to_either(head, "adc", true);
    else if ((dbl & 0b1111110000111000)
                 == 0b1000000000010000) pc += proc_imm_to_rm(head, "adc", SIGNED, true);
    else if ((top & 0b11111110)
                 == 0b00010100) pc += proc_imm_to_accum(head, "adc");
    else if ((dbl & 0b1111111000111000)
                 == 0b1111111000000000) pc += proc_rm(head, "inc", true);
    else if ((top & 0b11111000)
                 == 0b01000000) pc += proc_reg(head, "inc");
    else if ((top & 0b11111100)
                 == 0b00101000) pc += proc_rm_and_reg_to_either(head, "sub", true);
    else if ((dbl & 0b1111110000111000)
                 == 0b1000000000101000) pc += proc_imm_to_rm(head, "sub", SIGNED, true);
    else if ((top & 0b11111110)
                 == 0b00101100) pc += proc_imm_to_accum(head, "sub");
    else if ((top & 0b11111100)
                 == 0b00011000) pc += proc_rm_and_reg_to_either(head, "ssb", true);
    else if ((dbl & 0b1111110000111000)
                 == 0b1000000000011000) pc += proc_imm_to_rm(head, "ssb", SIGNED, true);
    // else if ((top & 0b11111110)
    //              == 0b0001110) pc += ssb_3(head);
    else if ((dbl & 0b1111111000111000)
                 == 0b1111111000001000) pc += proc_rm(head, "dec", true);
    else if ((top & 0b11111000)
                 == 0b01001000) pc += proc_reg(head, "dec");
    else if ((dbl & 0b1111111000111000)
                 == 0b1111011000011000) pc += proc_rm(head, "neg", true);
    else if ((top & 0b11111100)
                 == 0b00111000) pc += proc_rm_and_reg_to_either(head, "cmp", true);
    else if ((dbl & 0b1111110000111000)
                 == 0b1000000000111000) pc += proc_imm_to_rm(head, "cmp", SIGNED, true);
    else if ((top & 0b11111110)
                 == 0b00111100) pc += proc_imm_to_accum(head, "cmp");
    else if ((dbl & 0b1111111000111000)
                 == 0b1111011000100000) pc += proc_rm(head, "mul", true);
    else if ((dbl & 0b1111111000111000)
                 == 0b1111011000110000) pc += proc_rm(head, "div", true);
    else if ((top & 0b11111111)
                 == 0b10011000) pc += proc_single(head, "cbw");
    else if ((top & 0b11111111)
                 == 0b10011001) pc += proc_single(head, "cwd");
    // LOGIC
    else if ((dbl & 0b1111111000111000)
                 == 0b1111001000010000) pc += proc_logic(head, "not");
    else if ((dbl & 0b1111110000111000)
                 == 0b1101000000100000) pc += proc_logic(head, "shl", true);
    else if ((dbl & 0b1111110000111000)
                 == 0b1101000000101000) pc += proc_logic(head, "shr", true);
    else if ((dbl & 0b1111110000111000)
                 == 0b1101000000111000) pc += proc_logic(head, "sar", true);
    else if ((dbl & 0b1111110000111000)
                 == 0b1101000000000000) pc += proc_logic(head, "rol", true);
    else if ((dbl & 0b1111110000111000)
                 == 0b1101000000001000) pc += proc_logic(head, "ror", true);
    else if ((dbl & 0b1111110000111000)
                 == 0b1101000000010000) pc += proc_logic(head, "rcl", true);
    else if ((dbl & 0b1111110000111000)
                 == 0b1101000000011000) pc += proc_logic(head, "rcr", true);
    // AND
    else if ((top & 0b11111100)
                 == 0b00100000) pc += proc_rm_and_reg_to_either(head, "and", true);
    else if ((dbl & 0b1111111000111000)
                 == 0b1000000000100000) pc += proc_imm_to_rm(head, "and", UNSIGNED);
    else if ((top & 0b11111110)
                 == 0b00100100) pc += proc_imm_to_accum(head, "and");
    // TEST
    else if ((top & 0b11111110)
                 == 0b10000100) pc += proc_rm_and_reg_to_either(head, "test");
    else if ((dbl & 0b1111111000111000)
                 == 0b1111011000000000) pc += proc_imm_to_rm(head, "test", UNSIGNED);
    else if ((top & 0b11111110)
                 == 0b10101000) pc += proc_imm_to_accum(head, "test");
    // OR
    else if ((top & 0b11111100)
                 == 0b00001000) pc += proc_rm_and_reg_to_either(head, "or", true);
    else if ((dbl & 0b1111111000111000)
                 == 0b1000000000001000) pc += proc_imm_to_rm(head, "or", UNSIGNED);
    else if ((top & 0b11111110)
                 == 0b00001100) pc += proc_imm_to_accum(head, "or");
    // XOR
    else if ((top & 0b11111100)
                 == 0b00110000) pc += proc_rm_and_reg_to_either(head, "xor", true);
    else if ((dbl & 0b1111111000111000)
                 == 0b1000000000110000) pc += proc_imm_to_rm(head, "xor", UNSIGNED);
    else if ((top & 0b11111110)
                 == 0b00110100) pc += proc_imm_to_accum(head, "xor");
    // STRING MANIPULATION
    else if ((top & 0b11111110)
                 == 0b11110010) pc += inst_rep(head);
    // else if ((top & 0b11111110)
    //              == 0b10100100) pc += proc_single(head, "movs");
    // else if ((top & 0b11111110)
    //              == 0b10100110) pc += proc_single(head, "cmps");
    // else if ((top & 0b11111110)
    //              == 0b10101110) pc += proc_single(head, "scas");
    // else if ((top & 0b11111110)
    //              == 0b10101100) pc += proc_single(head, "lods");
    // else if ((top & 0b11111110)
    //              == 0b10101010) pc += proc_single(head, "stos");
    // CONTROL TRANSFER
    else if ((top & 0b11111111)
                 == 0b11101000) pc += proc_jmp_direct_within_segment(head, "call", pc);
    else if ((dbl & 0b1111111100111000)
                 == 0b1111111100010000) pc += proc_jmp_indirect_within_segment(head, "call");
    else if ((top & 0b11111111)
                 == 0b11101001) pc += proc_jmp_direct_within_segment(head, "jmp", pc);
    else if ((top & 0b11111111)
                 == 0b11101011) pc += proc_jmp_direct_within_segment(head, "jmp short", pc, true);
    else if ((dbl & 0b1111111100111000)
                 == 0b1111111100100000) pc += proc_jmp_indirect_within_segment(head, "jmp");
    else if ((top & 0b11111111)
                 == 0b11000011) pc += proc_single(head, "ret");
    else if ((top & 0b11111111)
                 == 0b11000010) pc += proc_disp(head, "ret");
    else if ((top & 0b11111111)
                 == 0b01110100) pc += proc_branch(head, "je", pc);
    else if ((top & 0b11111111)
                 == 0b01111100) pc += proc_branch(head, "jl", pc);
    else if ((top & 0b11111111)
                 == 0b01111110) pc += proc_branch(head, "jle", pc);
    else if ((top & 0b11111111)
                 == 0b01110010) pc += proc_branch(head, "jb", pc);
    else if ((top & 0b11111111)
                 == 0b01110110) pc += proc_branch(head, "jbe", pc);
    else if ((top & 0b11111111)
                 == 0b01111010) pc += proc_branch(head, "jp", pc);
    else if ((top & 0b11111111)
                 == 0b01110000) pc += proc_branch(head, "jo", pc);
    else if ((top & 0b11111111)
                 == 0b01111000) pc += proc_branch(head, "js", pc);
    else if ((top & 0b11111111)
                 == 0b01110101) pc += proc_branch(head, "jne", pc);
    else if ((top & 0b11111111)
                 == 0b01111101) pc += proc_branch(head, "jnl", pc);
    else if ((top & 0b11111111)
                 == 0b01111111) pc += proc_branch(head, "jnle", pc);
    else if ((top & 0b11111111)
                 == 0b01110011) pc += proc_branch(head, "jnb", pc);
    else if ((top & 0b11111111)
                 == 0b01110111) pc += proc_branch(head, "jnbe", pc);
    else if ((top & 0b11111111)
                 == 0b01111011) pc += proc_branch(head, "jnp", pc);
    else if ((top & 0b11111111)
                 == 0b01110001) pc += proc_branch(head, "jno", pc);
    else if ((top & 0b11111111)
                 == 0b01111001) pc += proc_branch(head, "jns", pc);
    else if ((top & 0b11111111)
                 == 0b11100010) pc += proc_branch(head, "loop", pc);
    else if ((top & 0b11111111)
                 == 0b11100001) pc += proc_branch(head, "loopz", pc);
    else if ((top & 0b11111111)
                 == 0b11100000) pc += proc_branch(head, "loopnz", pc);

    else if ((top & 0b11111111)
                 == 0b11001101) pc += inst_int_1(head);

    else if ((top & 0b11111111)
                 == 0b11111000) pc += proc_single(head, "clc");
    else if ((top & 0b11111111)
                 == 0b11110101) pc += proc_single(head, "cmc");
    else if ((top & 0b11111111)
                 == 0b11111001) pc += proc_single(head, "stc");
    else if ((top & 0b11111111)
                 == 0b11111100) pc += proc_single(head, "cld");
    else if ((top & 0b11111111)
                 == 0b11111101) pc += proc_single(head, "std");
    else if ((top & 0b11111111)
                 == 0b11111010) pc += proc_single(head, "cli");
    else if ((top & 0b11111111)
                 == 0b11111011) pc += proc_single(head, "sti");
    else if ((top & 0b11111111)
                 == 0b11110100) pc += proc_single(head, "hlt");
    else if ((top & 0b11111111)
                 == 0b10011011) pc += proc_single(head, "wait");
    else if ((top & 0b11111000)
                 == 0b11011000) pc += proc_rm(head, "esc");
    else if ((top & 0b11111111)
                 == 0b10011011) pc += proc_single(head, "lock");
    else {
      cout << instruction_str(head, 1) << "************";
      pc++;
    }

    cout << endl;
  }
}

size_t Disassembler::inst_in_1(const char *head) {
  Inst inst(head);
  inst.w = head[0];

  const size_t len = 2;
  cout << instruction_str(head, len);
  cout << "in " << (inst.w ? "ax" : "al");
  cout << ", " << Disassembler::data_str_narrow(head[1]);

  return len;
}

size_t Disassembler::inst_in_2(const char *head) {
  Inst inst(head);
  inst.w = head[0];

  const size_t len = 1;
  cout << instruction_str(head, len);
  cout << "in " << (inst.w ? "ax" : "al") << ", dx";

  return len;
}

size_t Disassembler::inst_lea(const char *head) {
  Inst inst(head);
  inst.set_mod_sec();

  const size_t len = inst.get_inst_len();
  cout << inst.get_inst_str("lea");
  cout << inst.get_reg_name() << ", " << inst.get_rm_str();

  return len;
}

size_t Disassembler::inst_int_1(const char *head) {
  const unsigned char type = get_data_narrow(&head[1]);
  const size_t len = 2;
  cout << instruction_str(head, len) << "int ";
  cout << data_str_narrow(type);
  return len;
}

size_t Disassembler::inst_rep(const char *head) {
  const unsigned char z = 0b1 & head[0];
  const unsigned char ex = 0xff & head[1];
  const size_t len = 2;
  cout << instruction_str(head, len);
  cout << "rep " << (ex == 0xa5 ? "movsw" : "movsb");
  return len;
}

// templates

size_t Disassembler::proc_rm_and_reg_to_either(const char *head, const char *name, const bool d) {
  Inst inst(head);
  if (d) inst.d = head[0] >> 1;
  inst.w = head[0];
  inst.set_mod_sec();

  const size_t len = inst.get_inst_len();
  cout << inst.get_inst_str(name);
  cout << inst.get_dist_str();

  return len;
}

size_t Disassembler::proc_imm_to_rm(const char *head, const char *name, const bool sign, const bool s) {
  Inst inst(head);
  if (s) inst.s = head[0] >> 1;
  inst.w = head[0];
  inst.set_mod_sec();
  inst.set_data();

  const size_t len = inst.get_inst_len();
  string fixed_name(name);
  fixed_name += (!inst.w && inst.mod != 0b11) ? " byte" : "";
  cout << inst.get_inst_str(fixed_name.c_str());
  cout << inst.get_rm_str() << ", " << inst.get_data_str(sign);

  return len;
}

size_t Disassembler::proc_imm_to_reg(const char *head, const char *name) {
  Inst inst(head);
  inst.w = head[0] >> 3;
  inst.reg = head[0];
  inst.set_data();

  const size_t len = inst.get_inst_len();
  cout << inst.get_inst_str(name);
  cout << inst.get_reg_name() << ", " << inst.get_data_str();

  return len;
}

size_t Disassembler::proc_imm_to_accum(const char *head, const char *name) {
  Inst inst(head);
  inst.w = head[0];
  inst.set_data();

  const size_t len = inst.get_inst_len();
  cout << inst.get_inst_str(name);
  cout << inst.get_accumulator_str() << ", " << inst.get_data_str();

  return len;
}

size_t Disassembler::proc_rm(const char *head, const char *name, const bool w) {
  Inst inst(head);
  if (w) inst.w = head[0];
  inst.set_mod_sec();

  const size_t len = inst.get_inst_len();
  cout << inst.get_inst_str(name);
  cout << inst.get_rm_str();

  return len;
}

size_t Disassembler::proc_reg(const char *head, const char *name) {
  Inst inst(head);
  inst.reg = head[0];

  const size_t len = inst.get_inst_len();
  cout << inst.get_inst_str(name);
  cout << inst.get_reg_name();

  return len;
}

size_t Disassembler::proc_reg_with_accum(const char *head, const char *name) {
  Inst inst(head);
  inst.reg = head[0];

  const size_t len = inst.get_inst_len();
  cout << inst.get_inst_str(name);
  cout << inst.get_reg_name() << ", " << inst.get_accumulator_str();

  return len;
}

size_t Disassembler::proc_logic(const char *head, const char *name, const bool v) {
  Inst inst(head);
  if (v) inst.v = head[0] >> 1;
  inst.w = head[0];
  inst.set_mod_sec();

  const size_t len = inst.get_inst_len();
  cout << inst.get_inst_str(name);
  cout << inst.get_rm_str();
  if (v) cout << ", " << (inst.v ? "[cl]" : "1");

  return len;
}

size_t Disassembler::proc_jmp_direct_within_segment(const char *head, const char *name, const size_t pc, const bool narrow) {
  short disp;
  if (narrow) {
    disp = static_cast<signed char>(get_data_narrow(&head[1]));
  } else {
    disp = static_cast<signed short>(get_data_wide(&head[1]));
  }
  const size_t len = narrow ? 2 : 3;
  cout << instruction_str(head, len) << name << " ";
  cout << data_str_wide(pc + len + disp);
  return len;
}

size_t Disassembler::proc_jmp_indirect_within_segment(const char *head, const char *name) {
  Inst inst(head);
  inst.set_mod_sec();

  const size_t len = inst.get_inst_len();
  cout << inst.get_inst_str(name);
  cout << inst.get_rm_str();

  return len;
}

size_t Disassembler::proc_branch(const char *head, const char *name, const size_t pc) {
  const char disp = get_data_narrow(&head[1]);
  const size_t len = 2;
  cout << instruction_str(head, len) << name << " ";
  cout << data_str_wide(pc + len + disp);
  return len;
}

size_t Disassembler::proc_single(const char *head, const char *name) {
  const size_t len = 1;
  cout << instruction_str(head, len) << name;
  return len;
}

size_t Disassembler::proc_disp(const char *head, const char *name, const bool narrow) {
  short disp;
  if (narrow) {
    disp = static_cast<signed char>(get_data_narrow(&head[1]));
  } else {
    disp = static_cast<signed short>(get_data_wide(&head[1]));
  }
  const size_t len = narrow ? 2 : 3;
  cout << instruction_str(head, len) << name << " ";
  cout << data_str_wide(disp);
  return len;
}

// statics

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

string Disassembler::data_str_wide(const unsigned short data, const bool nat, const bool sign) {
  return hex_str(sign ? static_cast<short>(data & 0xffff) : (data & 0xffff), nat ? 0 : 4);
}

string Disassembler::data_str_narrow(const unsigned char data, const bool nat, const bool sign) {
  return hex_str(sign ? static_cast<char>(data & 0xff) : (data & 0xff), nat ? 0 : 2);
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

string Disassembler::hex_str(int value, size_t w) {
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

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <cmath>
#include "interpreter.hpp"
#include "inst.hpp"
#include "util.hpp"
#include "consts.hpp"
#include "message.hpp"

using namespace std;

operation Interpreter::fn_mov = [](short d, short s, Flags &f) {
  return s;
};
operation Interpreter::fn_sub = [](short d, short s, Flags &f) {
  int result = d - s;
  f.o = (s < 0 ? SHRT_MAX - d : d - SHRT_MIN) < abs(s); 
  f.s = result < 0;
  f.z = result == 0;
  f.c = d < s;
  return result;
};
operation Interpreter::fn_xor = [](short d, short s, Flags &f) {
  int result = d ^ s;
  f.o = false;
  f.s = result < 0;
  f.z = result == 0;
  f.c = false;
  return result;
};
operation Interpreter::fn_add = [](short d, short s, Flags &f) {
  int result = d + s;
  f.o = (s > 0 ? SHRT_MAX - d : d - SHRT_MIN) < abs(s);
  f.s = result < 0;
  f.z = result == 0;
  f.c = (SHRT_MAX - d) < s;
  return result;
};
operation Interpreter::fn_cmp = [](short d, short s, Flags &f) {
  fn_sub(d, s, f);
  return d;
};
operation Interpreter::fn_jnb = [](short orig, short disp, Flags &f) {
  return !f.s ? disp : orig;
};
operation Interpreter::fn_test = [](short src1, short src2, Flags &f) {
  int result = src1 & src2;
  f.o = 0;
  f.s = result < 0;
  f.z = result == 0;
  f.c = 0;
  return 0;
};

void Interpreter::interpret() {
  cout << " AX   BX   CX   DX   SP   BP   SI   DI  FLAG  IP" << endl;

  while (pc < header.a_text) {
    cout << reg.str() << ' ';
    cout << flags.str() << ' ';
    cout << util::line_number_str(pc);

    const char *head = &text_seg[pc];
    const unsigned short top = head[0] & 0xff;
    const unsigned short dbl = (head[0] << 8 & 0xff00) + (head[1] & 0xff);

    if      ((top & 0b11111100)
                 == 0b10001000) pc += proc_rm_and_reg_to_either(head, "mov", fn_mov, true);
    else if ((dbl & 0b1111111000111000)
                 == 0b1100011000000000) pc += proc_imm_to_rm(head, "mov", fn_mov, SIGNED);
    else if ((top & 0b11110000)
                 == 0b10110000) pc += proc_imm_to_reg(head, "mov", fn_mov);
    // else if ((dbl & 0b1111111100111000)
    //              == 0b1111111100110000) pc += proc_rm(head, "push");
    // else if ((top & 0b11111000)
    //              == 0b01010000) pc += proc_reg(head, "push");
    // else if ((dbl & 0b1111111100111000)
    //              == 0b1000111100000000) pc += proc_rm(head, "pop");
    // else if ((top & 0b11111000)
    //              == 0b01011000) pc += proc_reg(head, "pop");
    // else if ((top & 0b11111110)
    //              == 0b10000110) pc += proc_rm_and_reg_to_either(head, "xchg");
    // else if ((top & 0b11111000)
    //              == 0b10010000) pc += proc_reg_with_accum(head, "xchg");
    // else if ((top & 0b11111110)
    //              == 0b11100100) pc += inst_in_1(head);
    // else if ((top & 0b11111110)
    //              == 0b11101100) pc += inst_in_2(head);
    else if ((top & 0b11111111)
                 == 0b10001101) pc += inst_lea(head);
    else if ((top & 0b11111100)
                 == 0b00000000) pc += proc_rm_and_reg_to_either(head, "add", fn_add, true);
    else if ((dbl & 0b1111110000111000)
                 == 0b1000000000000000) pc += proc_imm_to_rm(head, "add", fn_add, SIGNED, true);
    else if ((top & 0b11111110)
                 == 0b00000100) pc += proc_imm_to_accum(head, "add", fn_add);
    // else if ((top & 0b11111100)
    //              == 0b00010000) pc += proc_rm_and_reg_to_either(head, "adc", true);
    // else if ((dbl & 0b1111110000111000)
    //              == 0b1000000000010000) pc += proc_imm_to_rm(head, "adc", SIGNED, true);
    // else if ((top & 0b11111110)
    //              == 0b00010100) pc += proc_imm_to_accum(head, "adc");
    // else if ((dbl & 0b1111111000111000)
    //              == 0b1111111000000000) pc += proc_rm(head, "inc", true);
    // else if ((top & 0b11111000)
    //              == 0b01000000) pc += proc_reg(head, "inc");
    // else if ((top & 0b11111100)
    //              == 0b00101000) pc += proc_rm_and_reg_to_either(head, "sub", true);
    else if ((dbl & 0b1111110000111000)
                 == 0b1000000000101000) pc += proc_imm_to_rm(head, "sub", fn_sub, SIGNED, true);
    // else if ((top & 0b11111110)
    //              == 0b00101100) pc += proc_imm_to_accum(head, "sub");
    // else if ((top & 0b11111100)
    //              == 0b00011000) pc += proc_rm_and_reg_to_either(head, "ssb", true);
    // else if ((dbl & 0b1111110000111000)
    //              == 0b1000000000011000) pc += proc_imm_to_rm(head, "ssb", SIGNED, true);
    // // else if ((top & 0b11111110)
    // //              == 0b0001110) pc += ssb_3(head);
    // else if ((dbl & 0b1111111000111000)
    //              == 0b1111111000001000) pc += proc_rm(head, "dec", true);
    // else if ((top & 0b11111000)
    //              == 0b01001000) pc += proc_reg(head, "dec");
    // else if ((dbl & 0b1111111000111000)
    //              == 0b1111011000011000) pc += proc_rm(head, "neg", true);
    else if ((top & 0b11111100)
                 == 0b00111000) pc += proc_rm_and_reg_to_either(head, "cmp", fn_cmp, true);
    else if ((dbl & 0b1111110000111000)
                 == 0b1000000000111000) pc += proc_imm_to_rm(head, "cmp", fn_cmp, SIGNED, true);
    else if ((top & 0b11111110)
                 == 0b00111100) pc += proc_imm_to_accum(head, "cmp", fn_cmp);
    // else if ((dbl & 0b1111111000111000)
    //              == 0b1111011000100000) pc += proc_rm(head, "mul", true);
    // else if ((dbl & 0b1111111000111000)
    //              == 0b1111011000110000) pc += proc_rm(head, "div", true);
    // else if ((top & 0b11111111)
    //              == 0b10011000) pc += proc_single(head, "cbw");
    // else if ((top & 0b11111111)
    //              == 0b10011001) pc += proc_single(head, "cwd");
    // // LOGIC
    // else if ((dbl & 0b1111111000111000)
    //              == 0b1111001000010000) pc += proc_logic(head, "not");
    // else if ((dbl & 0b1111110000111000)
    //              == 0b1101000000100000) pc += proc_logic(head, "shl", true);
    // else if ((dbl & 0b1111110000111000)
    //              == 0b1101000000101000) pc += proc_logic(head, "shr", true);
    // else if ((dbl & 0b1111110000111000)
    //              == 0b1101000000111000) pc += proc_logic(head, "sar", true);
    // else if ((dbl & 0b1111110000111000)
    //              == 0b1101000000000000) pc += proc_logic(head, "rol", true);
    // else if ((dbl & 0b1111110000111000)
    //              == 0b1101000000001000) pc += proc_logic(head, "ror", true);
    // else if ((dbl & 0b1111110000111000)
    //              == 0b1101000000010000) pc += proc_logic(head, "rcl", true);
    // else if ((dbl & 0b1111110000111000)
    //              == 0b1101000000011000) pc += proc_logic(head, "rcr", true);
    // // AND
    // else if ((top & 0b11111100)
    //              == 0b00100000) pc += proc_rm_and_reg_to_either(head, "and", true);
    // else if ((dbl & 0b1111111000111000)
    //              == 0b1000000000100000) pc += proc_imm_to_rm(head, "and", UNSIGNED);
    // else if ((top & 0b11111110)
    //              == 0b00100100) pc += proc_imm_to_accum(head, "and");
    // TEST
    else if ((top & 0b11111110)
                 == 0b10000100) pc += proc_rm_and_reg_to_either(head, "test", fn_test);
    else if ((dbl & 0b1111111000111000)
                 == 0b1111011000000000) pc += proc_imm_to_rm(head, "test", fn_test, UNSIGNED);
    else if ((top & 0b11111110)
                 == 0b10101000) pc += proc_imm_to_accum(head, "test", fn_test);
    // // OR
    // else if ((top & 0b11111100)
    //              == 0b00001000) pc += proc_rm_and_reg_to_either(head, "or", true);
    // else if ((dbl & 0b1111111000111000)
    //              == 0b1000000000001000) pc += proc_imm_to_rm(head, "or", UNSIGNED);
    // else if ((top & 0b11111110)
    //              == 0b00001100) pc += proc_imm_to_accum(head, "or");
    // XOR
    else if ((top & 0b11111100)
                 == 0b00110000) pc += proc_rm_and_reg_to_either(head, "xor", fn_xor, true);
    else if ((dbl & 0b1111111000111000)
                 == 0b1000000000110000) pc += proc_imm_to_rm(head, "xor", fn_xor, UNSIGNED);
    // else if ((top & 0b11111110)
    //              == 0b00110100) pc += proc_imm_to_accum(head, "xor", fn_xor);
    // // STRING MANIPULATION
    // else if ((top & 0b11111110)
    //              == 0b11110010) pc += inst_rep(head);
    // // else if ((top & 0b11111110)
    // //              == 0b10100100) pc += proc_single(head, "movs");
    // // else if ((top & 0b11111110)
    // //              == 0b10100110) pc += proc_single(head, "cmps");
    // // else if ((top & 0b11111110)
    // //              == 0b10101110) pc += proc_single(head, "scas");
    // // else if ((top & 0b11111110)
    // //              == 0b10101100) pc += proc_single(head, "lods");
    // // else if ((top & 0b11111110)
    // //              == 0b10101010) pc += proc_single(head, "stos");
    // // CONTROL TRANSFER
    // else if ((top & 0b11111111)
    //              == 0b11101000) pc += proc_jmp_direct_within_segment(head, "call", pc);
    // else if ((dbl & 0b1111111100111000)
    //              == 0b1111111100010000) pc += proc_jmp_indirect_within_segment(head, "call");
    // else if ((top & 0b11111111)
    //              == 0b11101001) pc += proc_jmp_direct_within_segment(head, "jmp", pc);
    // else if ((top & 0b11111111)
    //              == 0b11101011) pc += proc_jmp_direct_within_segment(head, "jmp short", pc, true);
    // else if ((dbl & 0b1111111100111000)
    //              == 0b1111111100100000) pc += proc_jmp_indirect_within_segment(head, "jmp");
    // else if ((top & 0b11111111)
    //              == 0b11000011) pc += proc_single(head, "ret");
    // else if ((top & 0b11111111)
    //              == 0b11000010) pc += proc_disp(head, "ret");
    // else if ((top & 0b11111111)
    //              == 0b01110100) pc += proc_branch(head, "je", pc);
    // else if ((top & 0b11111111)
    //              == 0b01111100) pc += proc_branch(head, "jl", pc);
    // else if ((top & 0b11111111)
    //              == 0b01111110) pc += proc_branch(head, "jle", pc);
    // else if ((top & 0b11111111)
    //              == 0b01110010) pc += proc_branch(head, "jb", pc);
    // else if ((top & 0b11111111)
    //              == 0b01110110) pc += proc_branch(head, "jbe", pc);
    // else if ((top & 0b11111111)
    //              == 0b01111010) pc += proc_branch(head, "jp", pc);
    // else if ((top & 0b11111111)
    //              == 0b01110000) pc += proc_branch(head, "jo", pc);
    // else if ((top & 0b11111111)
    //              == 0b01111000) pc += proc_branch(head, "js", pc);
    // else if ((top & 0b11111111)
    //              == 0b01110101) pc += proc_branch(head, "jne", pc);
    // else if ((top & 0b11111111)
    //              == 0b01111101) pc += proc_branch(head, "jnl", pc);
    // else if ((top & 0b11111111)
    //              == 0b01111111) pc += proc_branch(head, "jnle", pc);
    else if ((top & 0b11111111)
                 == 0b01110011) pc += proc_branch(head, "jnb", fn_jnb);
    // else if ((top & 0b11111111)
    //              == 0b01110111) pc += proc_branch(head, "jnbe", pc);
    // else if ((top & 0b11111111)
    //              == 0b01111011) pc += proc_branch(head, "jnp", pc);
    // else if ((top & 0b11111111)
    //              == 0b01110001) pc += proc_branch(head, "jno", pc);
    // else if ((top & 0b11111111)
    //              == 0b01111001) pc += proc_branch(head, "jns", pc);
    // else if ((top & 0b11111111)
    //              == 0b11100010) pc += proc_branch(head, "loop", pc);
    // else if ((top & 0b11111111)
    //              == 0b11100001) pc += proc_branch(head, "loopz", pc);
    // else if ((top & 0b11111111)
    //              == 0b11100000) pc += proc_branch(head, "loopnz", pc);

    else if ((top & 0b11111111)
                 == 0b11001101) pc += inst_int_1(head);

    // else if ((top & 0b11111111)
    //              == 0b11111000) pc += proc_single(head, "clc");
    // else if ((top & 0b11111111)
    //              == 0b11110101) pc += proc_single(head, "cmc");
    // else if ((top & 0b11111111)
    //              == 0b11111001) pc += proc_single(head, "stc");
    // else if ((top & 0b11111111)
    //              == 0b11111100) pc += proc_single(head, "cld");
    // else if ((top & 0b11111111)
    //              == 0b11111101) pc += proc_single(head, "std");
    // else if ((top & 0b11111111)
    //              == 0b11111010) pc += proc_single(head, "cli");
    // else if ((top & 0b11111111)
    //              == 0b11111011) pc += proc_single(head, "sti");
    // else if ((top & 0b11111111)
    //              == 0b11110100) pc += proc_single(head, "hlt");
    // else if ((top & 0b11111111)
    //              == 0b10011011) pc += proc_single(head, "wait");
    // else if ((top & 0b11111000)
    //              == 0b11011000) pc += proc_rm(head, "esc");
    // else if ((top & 0b11111111)
    //              == 0b10011011) pc += proc_single(head, "lock");

    else {
      cout << util::instruction_str(head, 1) << "************";
      pc++;
    }

    cout << endl;
  }
}

size_t Interpreter::proc_rm_and_reg_to_either(const char *head, const char *name, operation op, const bool d) {
  Inst inst(this);
  if (d) inst.d = head[0] >> 1;
  inst.w = head[0];
  inst.set_mod_sec();

  const size_t len = inst.get_inst_len();
  cout << inst.get_inst_str(name);
  cout << inst.get_dist_str();

  if (inst.d) {
    const int d = inst.get_reg_value();
    const int s = inst.get_rm_value();
    const int data = op(d, s, flags);
    inst.put_value_reg(data);
    cout << "; " << hex << d << ", " << s << ": " << data;
  } else {
    const int d = inst.get_rm_value();
    const int s = inst.get_reg_value();
    const int data = op(d, s, flags);
    inst.put_value_rm(data);
    cout << "; " << hex << d << ", " << s << ": " << data;
  }

  return len;
}

size_t Interpreter::proc_imm_to_rm(const char *head, const char *name, operation op, const bool sign, const bool s) {
  Inst inst(this);
  if (s) inst.s = head[0] >> 1;
  inst.w = head[0];
  inst.set_mod_sec();
  inst.set_data();

  const size_t len = inst.get_inst_len();
  string fixed_name(name);
  fixed_name += (!inst.w && inst.mod != 0b11) ? " byte" : "";
  cout << inst.get_inst_str(fixed_name.c_str());
  cout << inst.get_rm_str() << ", " << inst.get_data_str(sign);

  const int data = op(inst.get_rm_value(), inst.get_data_value(), flags);
  inst.put_value_rm(data);

  return len;
}

size_t Interpreter::proc_imm_to_reg(const char *head, const char *name, operation op) {
  Inst inst(this);
  inst.w = head[0] >> 3;
  inst.reg = head[0];
  inst.set_data();

  const size_t len = inst.get_inst_len();
  cout << inst.get_inst_str(name);
  cout << inst.get_reg_name() << ", " << inst.get_data_str();

  const int data = op(inst.get_reg_value(), inst.get_data_value(), flags);
  inst.put_value_reg(data);

  return len;
}

size_t Interpreter::proc_imm_to_accum(const char *head, const char *name, operation op) {
  Inst inst(this);
  inst.w = head[0];
  inst.set_data();

  const size_t len = inst.get_inst_len();
  cout << inst.get_inst_str(name);
  cout << inst.get_accumulator_str() << ", " << inst.get_data_str();

  const int data = op(inst.get_accum_value(), inst.get_data_value(), flags);
  inst.put_accum_value(data);

  return len;
}

size_t Interpreter::proc_branch(const char *head, const char *name, operation op) {
  const char disp = util::get_data_narrow(&head[1]);
  const size_t len = 2;
  cout << util::instruction_str(head, len) << name << " ";
  cout << util::data_str_wide(pc + len + disp);
  return op(len, len + disp, flags);
}

size_t Interpreter::inst_lea(const char *head) {
  Inst inst(this);
  inst.set_mod_sec();

  const size_t len = inst.get_inst_len();
  cout << inst.get_inst_str("lea");
  cout << inst.get_reg_name() << ", " << inst.get_rm_str();

  const int ea = inst.get_ea_value();
  inst.put_value_reg(ea);

  return len;
}

size_t Interpreter::inst_int_1(const char *head) {
  const unsigned char type = util::get_data_narrow(&head[1]);
  const size_t len = 2;
  cout << util::instruction_str(head, len) << "int ";
  cout << util::data_str_narrow(type);

  Message *m = reinterpret_cast<Message *>(&data_seg[reg.b.x]);

  switch (m->m_type) {
    case 1: // exit
      cout << endl << "<exit(" << m->m1_i1 << ")>";
      cout << endl << flush;
      exit(m->m1_i1);
      break;
    case 4: // write
      cout << endl << "<write(";
      cout << m->m1_i1 << ", ";
      cout << "0x" << util::hex_str(m->m1_p1, sizeof(m->m1_p1) * 2) << ", ";
      cout << m->m1_i2 << ")>";
      cout << endl << flush;
      int ret = write(m->m1_i1, &data_seg[m->m1_p1], m->m1_i2);
      cout << endl << "=> " << ret;
      break;
  }

  return len;
}

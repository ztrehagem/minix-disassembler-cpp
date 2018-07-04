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

operation Interpreter::fn_mov = [](Machine *m, short d, short s) {
  return s;
};
operation Interpreter::fn_push = [](Machine *m, short is_wide, short value) {
  m->reg.sp -= is_wide ? 2 : 1;
  is_wide ? m->write_data_16(m->reg.sp, value) : m->write_data_8(m->reg.sp, value);
  cout << " push:" << hex << m->reg.sp << "(" << value << ")";
  return value;
};
operation Interpreter::fn_pop = [](Machine *m, short is_wide, short _) {
  unsigned short result = is_wide ? m->read_data_16(m->reg.sp) : m->read_data_8(m->reg.sp);
  cout << " pop:" << hex << m->reg.sp << "(" << result << ")";
  m->reg.sp += is_wide ? 2 : 1;
  return result;
};
operation Interpreter::fn_sub = [](Machine *m, short d, short s) {
  int result = d - s;
  m->flags.o = (s < 0 ? SHRT_MAX - d : d - SHRT_MIN) < abs(s); 
  m->flags.s = result < 0;
  m->flags.z = result == 0;
  m->flags.c = (d > 0 && d < s);
  return result;
};
operation Interpreter::fn_dec = [](Machine *m, short is_wide, short value) {
  int result = value - 1;
  m->flags.o = value == SHRT_MIN;
  m->flags.s = result < 0;
  m->flags.z = result == 0;
  // m->flags.c = value == 0; // XXX
  m->flags.c = false;
  return result;
};
operation Interpreter::fn_test = [](Machine *m, short src1, short src2) {
  int result = src1 & src2;
  m->flags.o = 0;
  m->flags.s = result < 0;
  m->flags.z = result == 0;
  m->flags.c = 0;
  return 0;
};
operation Interpreter::fn_or = [](Machine *m, short d, short s) {
  int result = d | s;
  m->flags.o = false;
  m->flags.s = result < 0;
  m->flags.z = result == 0;
  m->flags.c = false;
  return result;
};
operation Interpreter::fn_xor = [](Machine *m, short d, short s) {
  int result = d ^ s;
  m->flags.o = false;
  m->flags.s = result < 0;
  m->flags.z = result == 0;
  m->flags.c = false;
  return result;
};
operation Interpreter::fn_add = [](Machine *m, short d, short s) {
  int result = d + s;
  m->flags.o = (s > 0 ? SHRT_MAX - d : d - SHRT_MIN) < abs(s);
  m->flags.s = result < 0;
  m->flags.z = result == 0;
  m->flags.c = (SHRT_MAX - d) < s;
  return result;
};
operation Interpreter::fn_cmp = [](Machine *m, short d, short s) {
  fn_sub(m, d, s);
  return d;
};
operation Interpreter::fn_cbw = [](Machine *m, short _, short len) {
  m->reg.a.x = (char)m->reg.a.hl.l;
  return 0;
};
operation Interpreter::fn_call = [](Machine *m, short disp, short len) {
  fn_push(m, true, m->get_pc() + len);
  unsigned short pos = m->get_pc() + disp;
  cout << " call:" << hex << pos << "+len";
  m->set_pc(pos);
  return 0;
};
operation Interpreter::fn_jump = [](Machine *m, short disp, short _) {
  unsigned short pos = m->get_pc() + disp;
  cout << " jump:" << hex << pos << "+len";
  m->set_pc(pos);
  return 0;
};
operation Interpreter::fn_ret = [](Machine *m, short disp, short len) {
  m->reg.sp += disp; // XXX 正しい？
  unsigned short pc = fn_pop(m, true, 0);
  cout << " ret:" << hex << pc << "+len";
  m->set_pc(pc - len);
  return 0;
};
operation Interpreter::fn_je = [](Machine *m, short orig, short disp) {
  m->set_pc(m->get_pc() + (m->flags.z ? disp : orig));
  return disp;
};
operation Interpreter::fn_jl = [](Machine *m, short orig, short disp) {
  m->set_pc(m->get_pc() + (m->flags.s ? disp : orig));
  return disp;
};
operation Interpreter::fn_jle = [](Machine *m, short orig, short disp) {
  m->set_pc(m->get_pc() + ((m->flags.s || m->flags.z) ? disp : orig));
  return disp;
};
operation Interpreter::fn_jb = [](Machine *m, short orig, short disp) {
  m->set_pc(m->get_pc() + (m->flags.s ? disp : orig));
  return disp;
};
operation Interpreter::fn_jbe = [](Machine *m, short orig, short disp) {
  m->set_pc(m->get_pc() + ((m->flags.s || m->flags.z) ? disp : orig));
  return disp;
};
operation Interpreter::fn_jp = [](Machine *m, short orig, short disp) {
  m->set_pc(m->get_pc() + (m->flags.z ? disp : orig));
  return disp;
};
operation Interpreter::fn_jo = [](Machine *m, short orig, short disp) {
  m->set_pc(m->get_pc() + (m->flags.o ? disp : orig));
  return disp;
};
operation Interpreter::fn_js = [](Machine *m, short orig, short disp) {
  m->set_pc(m->get_pc() + (m->flags.s ? disp : orig));
  return disp;
};
operation Interpreter::fn_jne = [](Machine *m, short orig, short disp) {
  m->set_pc(m->get_pc() + (!m->flags.z ? disp : orig));
  return disp;
};
operation Interpreter::fn_jnl = [](Machine *m, short orig, short disp) {
  m->set_pc(m->get_pc() + (!m->flags.z ? disp : orig));
  return disp;
};
operation Interpreter::fn_jnle = [](Machine *m, short orig, short disp) {
  m->set_pc(m->get_pc() + ((!m->flags.s && !m->flags.z) ? disp : orig));
  return disp;
};
operation Interpreter::fn_jnb = [](Machine *m, short orig, short disp) {
  m->set_pc(m->get_pc() + (!m->flags.s ? disp : orig));
  return disp;
};
operation Interpreter::fn_jnbe = [](Machine *m, short orig, short disp) {
  m->set_pc(m->get_pc() + ((!m->flags.s && !m->flags.z) ? disp : orig));
  return disp;
};
operation Interpreter::fn_jnp = [](Machine *m, short orig, short disp) {
  m->set_pc(m->get_pc() + (!m->flags.z ? disp : orig));
  return disp;
};
operation Interpreter::fn_jno = [](Machine *m, short orig, short disp) {
  m->set_pc(m->get_pc() + (!m->flags.o ? disp : orig));
  return disp;
};
operation Interpreter::fn_jns = [](Machine *m, short orig, short disp) {
  m->set_pc(m->get_pc() + (!m->flags.s ? disp : orig));
  return disp;
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
    else if ((dbl & 0b1111111100111000)
                 == 0b1111111100110000) pc += proc_rm(head, "push", fn_push);
    else if ((top & 0b11111000)
                 == 0b01010000) pc += proc_reg(head, "push", fn_push);
    else if ((dbl & 0b1111111100111000)
                 == 0b1000111100000000) pc += proc_rm(head, "pop", fn_pop);
    else if ((top & 0b11111000)
                 == 0b01011000) pc += proc_reg(head, "pop", fn_pop);
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
    else if ((top & 0b11111100)
                 == 0b00101000) pc += proc_rm_and_reg_to_either(head, "sub", fn_sub, true);
    else if ((dbl & 0b1111110000111000)
                 == 0b1000000000101000) pc += proc_imm_to_rm(head, "sub", fn_sub, SIGNED, true);
    else if ((top & 0b11111110)
                 == 0b00101100) pc += proc_imm_to_accum(head, "sub", fn_sub);
    // else if ((top & 0b11111100)
    //              == 0b00011000) pc += proc_rm_and_reg_to_either(head, "ssb", true);
    // else if ((dbl & 0b1111110000111000)
    //              == 0b1000000000011000) pc += proc_imm_to_rm(head, "ssb", SIGNED, true);
    // // else if ((top & 0b11111110)
    // //              == 0b0001110) pc += ssb_3(head);
    else if ((dbl & 0b1111111000111000)
                 == 0b1111111000001000) pc += proc_rm(head, "dec", fn_dec, true);
    else if ((top & 0b11111000)
                 == 0b01001000) pc += proc_reg(head, "dec", fn_dec);
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
    else if ((top & 0b11111111)
                 == 0b10011000) pc += proc_single(head, "cbw", fn_cbw);
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
    // OR
    else if ((top & 0b11111100)
                 == 0b00001000) pc += proc_rm_and_reg_to_either(head, "or", fn_or, true);
    else if ((dbl & 0b1111111000111000)
                 == 0b1000000000001000) pc += proc_imm_to_rm(head, "or", fn_or, UNSIGNED);
    else if ((top & 0b11111110)
                 == 0b00001100) pc += proc_imm_to_accum(head, "or", fn_or);
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
    // CONTROL TRANSFER
    else if ((top & 0b11111111)
                 == 0b11101000) pc += proc_jmp_direct_within_segment(head, "call", fn_call);
    else if ((dbl & 0b1111111100111000)
                 == 0b1111111100010000) pc += proc_jmp_indirect_within_segment(head, "call", fn_call);
    else if ((top & 0b11111111)
                 == 0b11101001) pc += proc_jmp_direct_within_segment(head, "jmp", fn_jump);
    else if ((top & 0b11111111)
                 == 0b11101011) pc += proc_jmp_direct_within_segment(head, "jmp short", fn_jump, true);
    else if ((dbl & 0b1111111100111000)
                 == 0b1111111100100000) pc += proc_jmp_indirect_within_segment(head, "jmp", fn_jump);
    else if ((top & 0b11111111)
                 == 0b11000011) pc += proc_single(head, "ret", fn_ret);
    else if ((top & 0b11111111)
                 == 0b11000010) pc += proc_disp(head, "ret disp", fn_ret);
    else if ((top & 0b11111111)
                 == 0b01110100) pc += proc_branch(head, "je", fn_je);
    else if ((top & 0b11111111)
                 == 0b01111100) pc += proc_branch(head, "jl", fn_jl);
    else if ((top & 0b11111111)
                 == 0b01111110) pc += proc_branch(head, "jle", fn_jle);
    else if ((top & 0b11111111)
                 == 0b01110010) pc += proc_branch(head, "jb", fn_jb);
    else if ((top & 0b11111111)
                 == 0b01110110) pc += proc_branch(head, "jbe", fn_jbe);
    else if ((top & 0b11111111)
                 == 0b01111010) pc += proc_branch(head, "jp", fn_jp);
    else if ((top & 0b11111111)
                 == 0b01110000) pc += proc_branch(head, "jo", fn_jo);
    else if ((top & 0b11111111)
                 == 0b01111000) pc += proc_branch(head, "js", fn_js);
    else if ((top & 0b11111111)
                 == 0b01110101) pc += proc_branch(head, "jne", fn_jne);
    else if ((top & 0b11111111)
                 == 0b01111101) pc += proc_branch(head, "jnl", fn_jnl);
    else if ((top & 0b11111111)
                 == 0b01111111) pc += proc_branch(head, "jnle", fn_jnle);
    else if ((top & 0b11111111)
                 == 0b01110011) pc += proc_branch(head, "jnb", fn_jnb);
    else if ((top & 0b11111111)
                 == 0b01110111) pc += proc_branch(head, "jnbe", fn_jnbe);
    else if ((top & 0b11111111)
                 == 0b01111011) pc += proc_branch(head, "jnp", fn_jnp);
    else if ((top & 0b11111111)
                 == 0b01110001) pc += proc_branch(head, "jno", fn_jno);
    else if ((top & 0b11111111)
                 == 0b01111001) pc += proc_branch(head, "jns", fn_jns);
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
    const int data = op(this, d, s);
    inst.put_reg_value(data);
    cout << "; " << hex << d << ", " << s << ": " << data;
  } else {
    const int d = inst.get_rm_value();
    const int s = inst.get_reg_value();
    const int data = op(this, d, s);
    inst.put_rm_value(data);
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
  bool is_wide = !(!inst.w && inst.mod != 0b11);
  string fixed_name(name);
  fixed_name += is_wide ? "" : " byte";
  cout << inst.get_inst_str(fixed_name.c_str());
  cout << inst.get_rm_str() << ", " << inst.get_data_str(sign);

  const int data = op(this, inst.get_rm_value(is_wide), inst.get_data_value());
  inst.put_rm_value(data, is_wide);

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

  const int data = op(this, inst.get_reg_value(), inst.get_data_value());
  inst.put_reg_value(data);

  return len;
}

size_t Interpreter::proc_imm_to_accum(const char *head, const char *name, operation op) {
  Inst inst(this);
  inst.w = head[0];
  inst.set_data();

  const size_t len = inst.get_inst_len();
  cout << inst.get_inst_str(name);
  cout << inst.get_accumulator_str() << ", " << inst.get_data_str();

  const int data = op(this, inst.get_accum_value(), inst.get_data_value());
  inst.put_accum_value(data);

  return len;
}

size_t Interpreter::proc_rm(const char *head, const char *name, operation op, const bool w) {
  Inst inst(this);
  if (w) inst.w = head[0];
  inst.set_mod_sec();

  const size_t len = inst.get_inst_len();
  cout << inst.get_inst_str(name);
  cout << inst.get_rm_str();

  const int data = op(this, inst.is_wide_data(), inst.get_rm_value());
  inst.put_rm_value(data);

  return len;
}

size_t Interpreter::proc_reg(const char *head, const char *name, operation op) {
  Inst inst(this);
  inst.reg = head[0];

  const size_t len = inst.get_inst_len();
  cout << inst.get_inst_str(name);
  cout << inst.get_reg_name();

  const int data = op(this, inst.is_wide_data(), inst.get_reg_value());
  inst.put_reg_value(data);

  return len;
}

size_t Interpreter::proc_jmp_direct_within_segment(const char *head, const char *name, operation op, const bool narrow) {
  short disp;
  if (narrow) {
    disp = static_cast<signed char>(util::get_data_narrow(&head[1]));
  } else {
    disp = static_cast<signed short>(util::get_data_wide(&head[1]));
  }
  const size_t len = narrow ? 2 : 3;
  cout << util::instruction_str(head, len) << name << " ";
  cout << util::data_str_wide(pc + len + disp);

  op(this, disp, len);
  return len;
}

size_t Interpreter::proc_jmp_indirect_within_segment(const char *head, const char *name, operation op) {
  Inst inst(this);
  inst.set_mod_sec();

  const size_t len = inst.get_inst_len();
  cout << inst.get_inst_str(name);
  cout << inst.get_rm_str();

  short disp = inst.get_rm_value() - len - pc;
  op(this, disp, len);
  return len;
}

size_t Interpreter::proc_branch(const char *head, const char *name, operation op) {
  const char disp = util::get_data_narrow(&head[1]);
  const size_t len = 2;
  cout << util::instruction_str(head, len) << name << " ";
  cout << util::data_str_wide(pc + len + disp);
  op(this, 0, disp);
  return len;
}

size_t Interpreter::inst_lea(const char *head) {
  Inst inst(this);
  inst.set_mod_sec();

  const size_t len = inst.get_inst_len();
  cout << inst.get_inst_str("lea");
  cout << inst.get_reg_name() << ", " << inst.get_rm_str();

  const unsigned short ea = inst.get_ea_value();
  inst.put_reg_value(ea);

  return len;
}

size_t Interpreter::proc_single(const char *head, const char *name, operation op) {
  const size_t len = 1;
  cout << util::instruction_str(head, len) << name;

  op(this, 0, len);

  return len;
}

size_t Interpreter::proc_disp(const char *head, const char *name, operation op, const bool narrow) {
  short disp;
  if (narrow) {
    disp = static_cast<signed char>(util::get_data_narrow(&head[1]));
  } else {
    disp = static_cast<signed short>(util::get_data_wide(&head[1]));
  }
  const size_t len = narrow ? 2 : 3;
  cout << util::instruction_str(head, len) << name << " ";
  cout << util::data_str_wide(disp);

  op(this, disp, len);

  return len;
}

size_t Interpreter::inst_int_1(const char *head) {
  const unsigned char type = util::get_data_narrow(&head[1]);
  const size_t len = 2;
  cout << util::instruction_str(head, len) << "int ";
  cout << util::data_str_narrow(type);

  Message *m = reinterpret_cast<Message *>(&data_seg[reg.b.x]);

  cout << endl << "Message:";
  for(size_t i = 0; i < sizeof(Message); i++) {
    cout << " " << setfill('0') << setw(2) << hex << (int)read_data_8(reg.b.x + i);
  }

  cout << dec;

  switch (m->m_type) {
    case 1: // exit
      cout << endl << "<exit(" << m->m1_i1 << ")>";
      cout << endl << flush;
      exit(m->m1_i1);
      break;
    case 4: // write
      cout << endl << "<write(";
      cout << m->m1_i1 << ", ";
      cout << "0x" << util::hex_str((unsigned short)m->m1_p1, sizeof(m->m1_p1) * 2) << ", ";
      cout << m->m1_i2 << ")>";
      cout << endl << flush;
      int ret = write(m->m1_i1, &data_seg[m->m1_p1], m->m1_i2);
      m->m_type = ret;
      cout << endl << "=> " << ret;
      break;
  }

  reg.a.x = 0; // success for system calling

  return len;
}

#pragma once

#include <fstream>
#include <functional>
#include "machine.hpp"
#include "inst.hpp"

typedef std::function<short(Machine *, bool wide, short &dest, short &src)> operation;

class Interpreter : public Machine {
public:
  Interpreter(std::ifstream &ifs, short argc, char const *argv[]);
  void interpret();

private:
  static operation fn_mov;
  static operation fn_push;
  static operation fn_pop;
  static operation fn_xchg;
  static operation fn_inc;
  static operation fn_sub;
  static operation fn_dec;
  static operation fn_neg;
  static operation fn_and;
  static operation fn_test;
  static operation fn_or;
  static operation fn_xor;
  static operation fn_add;
  static operation fn_cmp;
  static operation fn_mul;
  static operation fn_div;
  static operation fn_cbw;
  static operation fn_cwd;
  static operation fn_shl;
  static operation fn_shr;
  static operation fn_sar;
  static operation fn_call;
  static operation fn_jump;
  static operation fn_ret;
  static operation fn_je;
  static operation fn_jl;
  static operation fn_jle;
  static operation fn_jb;
  static operation fn_jbe;
  static operation fn_jp;
  static operation fn_jo;
  static operation fn_js;
  static operation fn_jne;
  static operation fn_jnl;
  static operation fn_jnle;
  static operation fn_jnb;
  static operation fn_jnbe;
  static operation fn_jnp;
  static operation fn_jno;
  static operation fn_jns;

  size_t proc_rm_and_reg_to_either(const char *head, const char *name, operation, const bool d = false);
  size_t proc_imm_to_rm(const char *head, const char *name, operation, const bool sign, const bool s = false);
  size_t proc_imm_to_reg(const char *head, const char *name, operation);
  size_t proc_imm_to_accum(const char *head, const char *name, operation);
  size_t proc_rm(const char *head, const char *name, operation, bool w = false);
  size_t proc_reg(const char *head, const char *name, operation);
  size_t proc_reg_with_accum(const char *head, const char *name, operation);
  size_t proc_logic(const char *head, const char *name, operation, const bool v = false);
  size_t proc_jmp_direct_within_segment(const char *head, const char *name, operation, const bool narrow = false);
  size_t proc_jmp_indirect_within_segment(const char *head, const char *name, operation);
  size_t proc_branch(const char *head, const char *name, operation);
  size_t proc_single(const char *head, const char *name, operation);
  size_t proc_disp(const char *head, const char *name, operation, const bool narrow = false);
  size_t inst_lea(const char *head);
  size_t inst_int_1(const char *head);
};

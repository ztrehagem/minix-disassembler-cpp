#pragma once

#include <fstream>
#include <functional>
#include "machine.hpp"

using namespace std;

typedef function<short(Machine *, short, short)> operation;

class Interpreter : public Machine {
public:
  Interpreter(ifstream &ifs) : Machine(ifs) {};
  void interpret();

private:
  static operation fn_mov;
  static operation fn_push;
  static operation fn_pop;
  static operation fn_sub;
  static operation fn_xor;
  static operation fn_add;
  static operation fn_cmp;
  static operation fn_call;
  static operation fn_ret;
  static operation fn_jne;
  static operation fn_jnb;
  static operation fn_test;

  size_t proc_rm_and_reg_to_either(const char *head, const char *name, operation, const bool d = false);
  size_t proc_imm_to_rm(const char *head, const char *name, operation, const bool sign, const bool s = false);
  size_t proc_imm_to_reg(const char *head, const char *name, operation);
  size_t proc_imm_to_accum(const char *head, const char *name, operation);
  size_t proc_rm(const char *head, const char *name, operation, bool w = false);
  size_t proc_reg(const char *head, const char *name, operation);
  size_t proc_jmp_direct_within_segment(const char *head, const char *name, operation, const bool narrow = false);
  size_t proc_jmp_indirect_within_segment(const char *head, const char *name, operation);
  size_t proc_branch(const char *head, const char *name, operation);
  size_t proc_single(const char *head, const char *name, operation);
  size_t proc_disp(const char *head, const char *name, operation, const bool narrow = false);
  size_t inst_lea(const char *head);
  size_t inst_int_1(const char *head);
};

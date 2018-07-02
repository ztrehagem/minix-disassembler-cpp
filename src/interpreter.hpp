#pragma once

#include <fstream>
#include <functional>
#include "machine.hpp"

using namespace std;

typedef function<short(short, short, Flags &)> operation;

class Interpreter : public Machine {
public:
  Interpreter(ifstream &ifs) : Machine(ifs) {};
  void interpret();

private:
  static operation fn_mov;
  static operation fn_sub;
  static operation fn_xor;
  static operation fn_add;
  static operation fn_cmp;
  static operation fn_jne;
  static operation fn_jnb;
  static operation fn_test;

  size_t proc_rm_and_reg_to_either(const char *head, const char *name, operation, const bool d = false);
  size_t proc_imm_to_rm(const char *head, const char *name, operation, const bool sign, const bool s = false);
  size_t proc_imm_to_reg(const char *head, const char *name, operation);
  size_t proc_imm_to_accum(const char *head, const char *name, operation);
  size_t proc_branch(const char *head, const char *name, operation);
  size_t inst_lea(const char *head);
  size_t inst_int_1(const char *head);
};

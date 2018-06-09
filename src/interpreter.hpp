#pragma once

#include <fstream>
#include <functional>
#include "a.out.hpp"
#include "inst.hpp"
#include "reg.hpp"

using namespace std;

class Interpreter {
public:
  Interpreter(ifstream &ifs) : ifs(ifs) {};
  void interpret();

private:
  ifstream &ifs;
  struct exec header;
  char *text_seg;
  char *data_seg;
  struct Reg reg;

  static function<int(int, int)> fn_mov;
  static function<int(int, int)> fn_sub;

  void execute();

  size_t proc_rm_and_reg_to_either(const char *head, const char *name, const bool d, function<int(int, int)>);
  size_t proc_imm_to_rm(const char *head, const char *name, const bool sign, const bool s, function<int(int, int)>);
  size_t proc_imm_to_reg(const char *head, const char *name, function<int(int, int)>);
  size_t inst_int_1(const char *head);
};

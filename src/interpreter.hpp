#pragma once

#include <fstream>
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

  void execute();

  size_t proc_imm_to_reg(const char *head, const char *name);
  size_t inst_int_1(const char *head);
};

#pragma once

#include <fstream>
#include <string>
#include "machine.hpp"

class Disassembler : public Machine {
public:
  Disassembler(std::ifstream &ifs) : Machine(ifs) {};
  void disassemble();

private:
  size_t inst_in_1(const char *);
  size_t inst_in_2(const char *);
  size_t inst_lea(const char *);
  size_t inst_int_1(const char *);
  size_t inst_rep(const char *);

  size_t proc_rm_and_reg_to_either(const char *head, const char *name, const bool d = false);
  size_t proc_imm_to_rm(const char *head, const char *name, const bool sign, const bool s = false);
  size_t proc_imm_to_reg(const char *head, const char *name);
  size_t proc_imm_to_accum(const char *head, const char *name);
  size_t proc_rm(const char *head, const char *name, const bool w = false);
  size_t proc_reg(const char *head, const char *name);
  size_t proc_reg_with_accum(const char *head, const char *name);
  size_t proc_logic(const char *head, const char *name, const bool v = false);
  size_t proc_jmp_direct_within_segment(const char *head, const char *name, const size_t pc, const bool narrow = false);
  size_t proc_jmp_indirect_within_segment(const char *head, const char *name);
  size_t proc_branch(const char *head, const char *name, const size_t pc);
  size_t proc_single(const char *head, const char *name);
  size_t proc_disp(const char *head, const char *name, const bool narrow = false);
};

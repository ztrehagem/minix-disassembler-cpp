#pragma once

#include <fstream>
#include <string>
#include "a.out.hpp"
#include "inst.hpp"

using namespace std;

class Disassembler {
public:
  Disassembler(ifstream &ifs) : ifs(ifs) {};
  void disassemble();

  static unsigned short get_data_wide(const char *head);
  static unsigned char get_data_narrow(const char *head);
  static size_t get_extended_len(const Inst &);
  static string data_str_wide(const unsigned short data, const bool sign = false);
  static string data_str_narrow(const unsigned char data, const bool sign = false);
  static string line_number_str(const size_t);
  static string instruction_str(const char *head, const size_t len);
  static string hex_str(int value, size_t w = 0);

private:
  ifstream& ifs;
  struct exec header;

  void analyze_text(const char text[], const size_t len);
  size_t inst_in_1(const char *);
  size_t inst_in_2(const char *);
  size_t inst_lea(const char *);
  size_t inst_int_1(const char *);
  size_t inst_rep(const char *);

  size_t proc_rm_and_reg_to_either(const char *head, const char *name, const bool d = false);
  size_t proc_imm_to_rm(const char *head, const char *name, const bool s = false);
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

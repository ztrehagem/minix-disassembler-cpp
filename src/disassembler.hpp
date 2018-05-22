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
  static string data_str_wide(const unsigned short data, const bool as_natural = false);
  static string data_str_narrow(const unsigned char data, const bool as_natural = false);
  static string line_number_str(const size_t);
  static string instruction_str(const char *head, const size_t len);
  static string hex_str(const unsigned int value, const size_t w = 0);

private:
  ifstream& ifs;
  struct exec header;

  void analyze_text(const char text[], const size_t len);
  size_t inst_in_1(const char *);
  size_t inst_in_2(const char *);
  size_t inst_lea(const char *);
  size_t inc_1(const char *);
  size_t inc_2(const char *);
  size_t sub_2(const char *);
  size_t sub_3(const char *);
  size_t ssb_2(const char *);
  // size_t ssb_3(const char *);
  size_t dec_1(const char *);
  size_t dec_2(const char *);
  size_t neg_1(const char *);
  size_t cmp_1(const char *);
  size_t cmp_2(const char *);
  size_t cmp_3(const char *);
  size_t mul_1(const char *);
  size_t cbw_1(const char *);
  size_t cwd_1(const char *);
  size_t shl_1(const char *);
  size_t and_1(const char *);
  size_t and_2(const char *);
  size_t test_2(const char *);
  size_t or_1(const char *);
  size_t or_2(const char *);
  size_t or_3(const char *);
  size_t xor_1(const char *);
  size_t call_1(const char *, const size_t pc);
  size_t call_2(const char *);
  size_t jmp_1(const char *, const size_t pc);
  size_t jmp_2(const char *, const size_t pc);
  size_t ret_1(const char *);
  size_t je_1(const char *, const size_t pc);
  size_t jl_1(const char *, const size_t pc);
  size_t jle_1(const char *, const size_t pc);
  size_t jb_1(const char *, const size_t pc);
  size_t jbe_1(const char *, const size_t pc);
  size_t jp_1(const char *, const size_t pc);
  size_t jo_1(const char *, const size_t pc);
  size_t js_1(const char *, const size_t pc);
  size_t jne_1(const char *, const size_t pc);
  size_t jnl_1(const char *, const size_t pc);
  size_t jnle_1(const char *, const size_t pc);
  size_t jnb_1(const char *, const size_t pc);
  size_t jnbe_1(const char *, const size_t pc);
  size_t jnp_1(const char *, const size_t pc);
  size_t jno_1(const char *, const size_t pc);
  size_t jns_1(const char *, const size_t pc);
  // size_t loop_1(const char *, const size_t pc);
  // size_t loopz_1(const char *, const size_t pc);
  // size_t loopnz_1(const char *, const size_t pc);
  size_t int_1(const char *);
  size_t hlt_1(const char *);

  size_t proc_rm_and_reg_to_either(const char *head, const char *name);
  size_t proc_imm_to_rm(const char *head, const char *name, const bool s = false);
  size_t proc_imm_to_reg(const char *head, const char *name);
  size_t proc_imm_to_accum(const char *head, const char *name);
  size_t proc_rm(const char *head, const char *name, const bool w = false);
  size_t proc_reg(const char *head, const char *name);
  size_t proc_single(const char *head, const char *name);
};

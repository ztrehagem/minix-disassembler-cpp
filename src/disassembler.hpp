#pragma once

#include <fstream>
#include <string>
#include <sstream>
#include "a.out.hpp"

using namespace std;

struct Inst {
  unsigned s:1;
  unsigned v:1;
  unsigned w:1;
  unsigned d:1;
  unsigned reg:3;
  unsigned mod:2;
  unsigned rm:3;
  union {
    unsigned wide:16;
    unsigned narrow:8;
  } data;
  signed disp:16;
  size_t disp_size;
  size_t data_size;
  bool has_mod_sec;
  bool has_data_sec;

  Inst() {
    s = 0;
    v = 0;
    w = 1;
    d = 0;
    reg = 0;
    mod = 0;
    rm = 0;
    disp = 0;
    disp_size = 0;
    data_size = 0;
    has_mod_sec = false;
    has_data_sec = false;
  }

  void set_mod_sec(const char *head);
  void set_data(const char *head);
  bool is_wide_data();
  size_t get_inst_len();
  string get_reg_name(const bool is_rm = false);
  string get_rm_str();
  string get_data_str(const bool as_natural = false);
  string get_dist_str();
};

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
  size_t mov_1(const char *);
  size_t mov_3(const char *);
  size_t push_1(const char *);
  size_t push_2(const char *);
  size_t pop_2(const char *);
  size_t in_1(const char *);
  size_t in_2(const char *);
  size_t lea_1(const char *);
  size_t add_1(const char *);
  size_t add_2(const char *);
  size_t sub_2(const char *);
  size_t ssb_1(const char *);
  size_t dec_2(const char *);
  size_t neg_1(const char *);
  size_t cmp_2(const char *);
  size_t shl_1(const char *);
  size_t and_1(const char *);
  size_t test_2(const char *);
  size_t or_1(const char *);
  size_t or_3(const char *);
  size_t xor_1(const char *);
  size_t call_1(const char *, const size_t pc);
  size_t call_2(const char *);
  size_t jmp_1(const char *, const size_t pc);
  size_t jmp_2(const char *, const size_t pc);
  size_t ret_1(const char *);
  size_t je_1(const char *, const size_t pc);
  size_t jl_1(const char *, const size_t pc);
  size_t jne_1(const char *, const size_t pc);
  size_t jnl_1(const char *, const size_t pc);
  size_t jnb_1(const char *, const size_t pc);
  size_t int_1(const char *);
  size_t hlt_1(const char *);
};

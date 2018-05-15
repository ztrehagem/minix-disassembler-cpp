#pragma once

#include <fstream>
#include <string>
#include "a.out.hpp"

using namespace std;

struct Inst {
  unsigned s:1;
  unsigned w:1;
  unsigned d:1;
  unsigned reg:3;
  unsigned mod:2;
  unsigned rm:3;
  union {
    unsigned wide:16;
    unsigned narrow:8;
  } data;
};

class Disassembler {
public:
  Disassembler(ifstream &ifs) : ifs(ifs) {};
  void disassemble();

private:
  ifstream& ifs;
  struct exec header;
  size_t ti = 0;

  void analyze_text(const char text[], const size_t len);
  size_t mov_1(const char *);
  size_t mov_3(const char *);
  size_t push_2(const char *);
  size_t lea_1(const char *);
  size_t add_1(const char *);
  size_t cmp_2(const char *);
  size_t test_2(const char *);
  size_t xor_1(const char *);
  size_t call_1(const char *);
  size_t jne_1(const char *);
  size_t jnb_1(const char *);
  size_t int_1(const char *);
  string get_reg_name(const Inst &, const bool is_rm = false);
  string get_rm_string(const Inst &, const char *extended = nullptr);
  void set_data(Inst &, const char *head, const bool is_wide);
  unsigned short get_data_wide(const char *head);
  unsigned char get_data_narrow(const char *head);
  void print_data(const Inst &, const bool is_wide, const bool as_natural = false);
  void print_data_wide(const unsigned short data, const bool as_natural = false);
  void print_data_narrow(const unsigned char data, const bool as_natural = false);
  void print_line_number(const size_t);
  void print_bytes(const char *head, const size_t len);
  void print_byte(const char);
};

#pragma once

#include <fstream>
#include <string>
#include "a.out.hpp"

using namespace std;

struct Inst {
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

  void analyze_text(const char text[], const size_t len);
  size_t mov_1(const char *head);
  size_t mov_3(const char *head);
  size_t add_1(const char *head);
  size_t xor_1(const char *head);
  size_t int_1(const char *head);
  string get_reg_name(const Inst &, const bool is_rm = false);
  string get_rm_string(const Inst &);
  void print_data_wide(const unsigned short data);
  void print_data_narrow(const unsigned char data);
  void print_line_number(const size_t);
  void print_bytes(const char *head, const size_t len);
  void print_byte(const char);
};

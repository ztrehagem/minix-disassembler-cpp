#pragma once

#include <fstream>
#include <string>
#include "a.out.hpp"

using namespace std;

struct info {
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
  // unsigned char text_mov(const char *word);
  // unsigned char text_int(const char *word);
  // unsigned char text_add(const char *word);
  size_t mov_3(const char *head);
  size_t add_1(const char *head);
  size_t int_1(const char *head);
  string get_reg_name(const struct info &);
  string get_rm_string(const struct info &);
  void print_line_number(const size_t);
  void print_data_wide(const unsigned short data);
  void print_data_narrow(const unsigned char data);
  void print_bytes(const char *head, const size_t len);
  void print_byte(const char);
};

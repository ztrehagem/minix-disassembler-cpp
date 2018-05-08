#pragma once

#include <fstream>
#include "a.out.hpp"

using namespace std;

class Disassembler {
public:
  Disassembler(ifstream &ifs) : ifs(ifs) {};
  void disassemble();

private:
  ifstream& ifs;
  struct exec header;

  void analyze_text(const char text[], const size_t len);
  unsigned char text_mov(const char *word);
  unsigned char text_int(const char *word);
  unsigned char text_add(const char *word);
  void print_line_number(const size_t);
  void print_bytes(const char *head, const size_t len);
  void print_byte(const char);
};

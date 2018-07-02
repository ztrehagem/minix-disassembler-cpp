#pragma once

#include <fstream>
#include "a.out.hpp"
#include "reg.hpp"
#include "flags.hpp"

class Machine {
public:
  Machine(std::ifstream &ifs);
  ~Machine();

  struct exec &get_header();
  char *get_head(size_t offset = 0);
  char *get_data_seg();
  char read_data_8(size_t pos);
  short read_data_16(size_t pos);
  void write_data_8(size_t pos, char value);
  void write_data_16(size_t pos, short value);
  struct Reg reg;
  struct Flags flags;

protected:
  std::ifstream &ifs;
  struct exec header;
  char *text_seg;
  char *data_seg;
  size_t pc;

private:
  void print_header();
};
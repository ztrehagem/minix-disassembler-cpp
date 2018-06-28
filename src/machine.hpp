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
  struct Reg &get_reg();

protected:
  std::ifstream &ifs;
  struct exec header;
  char *text_seg;
  char *data_seg;
  struct Reg reg;
  struct Flags flags;
  size_t pc;

private:
  void print_header();
};
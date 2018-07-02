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
  unsigned short get_pc();
  void set_pc(unsigned short);
  char *get_head(unsigned short offset = 0);
  char *get_data_seg();
  unsigned char read_data_8(unsigned short pos);
  unsigned short read_data_16(unsigned short pos);
  void write_data_8(unsigned short pos, unsigned char value);
  void write_data_16(unsigned short pos, unsigned short value);
  struct Reg reg;
  struct Flags flags;

protected:
  std::ifstream &ifs;
  struct exec header;
  char *text_seg;
  char *data_seg;
  unsigned short pc;

private:
  void print_header();
};
#include <iostream>
#include "machine.hpp"

Machine::Machine(std::ifstream &ifs) : ifs(ifs) {
  pc = 0;

  ifs.read(reinterpret_cast<char *>(&header), sizeof(header));
  print_header();

  text_seg = new char[header.a_text];
  data_seg = new char[header.a_data];

  ifs.read(text_seg, header.a_text);
  ifs.read(data_seg, header.a_data);
};

Machine::~Machine() {
  delete[] text_seg;
  delete[] data_seg;
}

struct exec &Machine::get_header() {
  return header;
}

char *Machine::get_head(size_t offset) {
  return &text_seg[pc + offset];
}

char *Machine::get_data_seg() {
  return data_seg;
}

struct Reg &Machine::get_reg() {
  return reg;
}

void Machine::print_header() {
  std::cout << "a_text: " << header.a_text << std::endl;
  std::cout << "a_data: " << header.a_data << std::endl;
}
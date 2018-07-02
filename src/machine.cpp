#include <iostream>
#include "machine.hpp"

Machine::Machine(std::ifstream &ifs) : ifs(ifs) {
  pc = 0;

  ifs.read(reinterpret_cast<char *>(&header), sizeof(header));
  print_header();

  text_seg = new char[header.a_text];
  data_seg = new char[0xffff];

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

unsigned short Machine::get_pc() {
  return pc;
}

void Machine::set_pc(unsigned short pc) {
  this->pc = pc;
}

char *Machine::get_head(unsigned short offset) {
  return &text_seg[pc + offset];
}

char *Machine::get_data_seg() {
  return data_seg;
}

unsigned char Machine::read_data_8(unsigned short pos) {
  return data_seg[pos];
}

unsigned short Machine::read_data_16(unsigned short pos) {
  return (0xff & data_seg[pos]) + ((0xff & data_seg[pos + 1]) << 8);
}

void Machine::write_data_8(unsigned short pos, unsigned char value) {
  data_seg[pos] = value;
}

void Machine::write_data_16(unsigned short pos, unsigned short value) {
  data_seg[pos] = 0xff & value;
  data_seg[pos + 1] = 0xff & (value >> 8);
}

void Machine::print_header() {
  std::cout << "a_text: " << header.a_text << std::endl;
  std::cout << "a_data: " << header.a_data << std::endl;
}
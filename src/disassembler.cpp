#include <iostream>
#include <iomanip>
#include <fstream>
#include "disassembler.hpp"

using namespace std;

void Disassembler::disassemble() {
  ifs.read(reinterpret_cast<char *>(&header), sizeof(header));
  cout << "a_text = " << header.a_text << endl;

  char text[header.a_text];
  ifs.read(text, sizeof(text));
  analyze_text(text, sizeof(text));

  cout << endl;
}

void Disassembler::analyze_text(const char text[], const size_t len) {
  size_t i = 0;

  while (i < len) {
    print_line_number(i);

    const char *head = &text[i];

    if      ((*head >> 4 &     0b1111) ==     0b1011) i += mov_3(head);
    else if ((*head >> 2 &   0b111111) ==   0b000000) i += add_1(head);
    else if ((*head      & 0b11111111) == 0b11001101) i += int_1(head);
    else {
      print_byte(*head);
      cout << "\t is not implemented";
      i++;
    }

    cout << endl;
  }
}

size_t Disassembler::mov_3(const char *head) {
  struct info info;
  info.w = head[0] >> 3 & 0b1;
  info.reg = head[0] & 0b111;
  if (info.w) {
    info.data.wide = (head[1] & 0xff) + ((head[2] & 0xff) << 8);
  } else {
    info.data.narrow = head[1] & 0xff;
  }

  const size_t len = info.w ? 3 : 2;
  print_bytes(head, len);
  cout << "\t mov ";
  cout << get_reg_name(info) << ", ";

  if (info.w) {
    print_data_wide(info.data.wide);
  } else {
    print_data_narrow(info.data.narrow);
  }

  return len;
}

size_t Disassembler::add_1(const char *head) {
  struct info info;
  info.d = head[0] >> 1 & 0b1;
  info.w = head[0] & 0b1;
  info.mod = head[1] >> 6 & 0b11;
  info.reg = head[1] >> 3 & 0b111;
  info.rm = head[1] & 0b111;

  const size_t len = 2;
  print_bytes(head, len);
  cout << "\t add ";

  if (info.d) {
    cout << get_reg_name(info) << ", " << get_rm_string(info);
  } else {
    cout << get_rm_string(info) << ", " << get_reg_name(info);
  }

  return len;
}

size_t Disassembler::int_1(const char *head) {
  const size_t len = 2;
  print_bytes(head, len);
  cout << "\t int ";
  print_data_narrow(head[1]);
  return len;
}

string Disassembler::get_reg_name(const struct info &info) {
  switch (((info.w << 3) + info.reg) & 0b1111) {
    case 0b1000: return "ax";
    case 0b1001: return "cx";
    case 0b1010: return "dx";
    case 0b1011: return "bx";
    case 0b1100: return "sp";
    case 0b1101: return "bp";
    case 0b1110: return "si";
    case 0b1111: return "di";
    case 0b0000: return "al";
    case 0b0001: return "cl";
    case 0b0010: return "dl";
    case 0b0011: return "bl";
    case 0b0100: return "ah";
    case 0b0101: return "ch";
    case 0b0110: return "dh";
    case 0b0111: return "bh";
    default: return "";
  }
}

string Disassembler::get_rm_string(const struct info &info) {
  unsigned char disp = 0;

  switch (info.mod & 0b11) {
    case 0b11: return get_reg_name(info);
    case 0b00: disp = 0; break;
    case 0b10: return "[??]";
    case 0b01: return "[??]";
  }

  string ea;

  switch (info.rm & 0b111) {
    case 0b000: ea = "bx+si"; break;
    case 0b001: ea = "bx+di"; break;
    case 0b010: ea = "bp+si"; break;
    case 0b011: ea = "bp+di"; break;
    case 0b100: ea = "si"; break;
    case 0b101: ea = "di"; break;
    case 0b110: ea = "bp"; break;
    case 0b111: ea = "bx"; break;
  }

  if (disp != 0) {
    ea += "+" + to_string(disp);
  }

  return "[" + ea + "]";
}

void Disassembler::print_data_wide(const unsigned short data) {
  cout << setfill('0') << setw(4) << hex << static_cast<int>(data & 0xffff);
}

void Disassembler::print_data_narrow(const unsigned char data) {
  cout << setfill('0') << setw(2) << hex << static_cast<int>(data & 0xff);
}

void Disassembler::print_line_number(const size_t n) {
  cout << setfill('0') << setw(4) << hex << n << ": ";
}

void Disassembler::print_bytes(const char *head, const size_t len) {
  for (size_t i = 0; i < len; i++) {
    print_byte(head[i]);
  }
}

void Disassembler::print_byte(const char c) {
  cout << setfill('0') << setw(2) << hex << static_cast<int>(0xff & c);
}

#include <iostream>
#include <iomanip>
#include <fstream>
#include "disassembler.hpp"

using namespace std;

void Disassembler::disassemble() {
  ifs.read(reinterpret_cast<char *>(&header), sizeof(header));
  cout << "a_text = " << header.a_text << endl;

  char text[header.a_text];
  ifs.read(text, header.a_text);
  analyze_text(text, sizeof(text));

  cout << endl;
}

void Disassembler::analyze_text(const char text[], const size_t len) {
  size_t i = 0;

  while (i < len) {
    print_line_number(i);

    switch (0xff & text[i]) {
      case 0xbb: i += text_mov(&text[i]); break;
      case 0xcd: i += text_int(&text[i]); break;
      case 0x00: i += text_add(&text[i]); break;
      default:
        print_byte(text[i]);
        cout << " is not expected";
        i++;
    }
    cout << endl;
  }
}

unsigned char Disassembler::text_mov(const char *word) {
  const unsigned char wlen = 3;
  print_bytes(word, wlen);
  cout << "\t mov bx, ";
  print_byte(word[2]);
  print_byte(word[1]);
  return wlen;
}

unsigned char Disassembler::text_int(const char *word) {
  const unsigned char wlen = 2;
  print_bytes(word, wlen);
  cout << "\t int ";
  print_byte(word[1]);
  return wlen;
}

unsigned char Disassembler::text_add(const char *word) {
  const unsigned char wlen = 2;
  print_bytes(word, wlen);
  cout << "\t add [bx+si], al";
  return wlen;
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

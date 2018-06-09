#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include "interpreter.hpp"
#include "inst.hpp"
#include "util.hpp"

using namespace std;

void Interpreter::interpret() {
  ifs.read(reinterpret_cast<char *>(&header), sizeof(header));

  text_seg = new char[header.a_text];
  data_seg = new char[header.a_data];

  ifs.read(text_seg, header.a_text);
  ifs.read(data_seg, header.a_data);

  execute();

  delete[] text_seg;
  delete[] data_seg;
}

void Interpreter::execute() {
  size_t pc = 0;

  cout << " AX   BX   CX   DX   SP   BP   SI  DI" << endl;

  while (pc < header.a_text) {
    cout << util::reg_state_str(reg);
    cout << util::line_number_str(pc);

    const char *head = &text_seg[pc];
    const unsigned short top = head[0] & 0xff;
    const unsigned short dbl = (head[0] << 8 & 0xff00) + (head[1] & 0xff);

         if ((top & 0b11110000)
                 == 0b10110000) pc += proc_imm_to_reg(head, "mov");
    else if ((top & 0b11111111)
                 == 0b11001101) pc += inst_int_1(head);
    else {
      cout << util::instruction_str(head, 1) << "************";
      pc++;
    }

    cout << endl;
  }
}

size_t Interpreter::proc_imm_to_reg(const char *head, const char *name) {
  Inst inst(head);
  inst.w = head[0] >> 3;
  inst.reg = head[0];
  inst.set_data();

  const size_t len = inst.get_inst_len();
  cout << inst.get_inst_str(name);
  cout << inst.get_reg_name() << ", " << inst.get_data_str();

  const int data = inst.get_data_value();
  inst.put_value_reg(reg, data);

  return len;
}

size_t Interpreter::inst_int_1(const char *head) {
  const unsigned char type = util::get_data_narrow(&head[1]);
  const size_t len = 2;
  cout << util::instruction_str(head, len) << "int ";
  cout << util::data_str_narrow(type);

  const unsigned short *args = reinterpret_cast<unsigned short *>(&data_seg[reg.b.x]);

  switch (args[1]) {
    case 1: // exit
      cout << endl << "<exit(" << 0 << ")>";
      cout << endl << flush;
      exit(0);
      break;
    case 4: // write
      cout << endl << "<write(";
      cout << args[2] << ", ";
      cout << "0x" << util::hex_str(args[5], 4) << ", ";
      cout << args[3] << ")>";
      cout << endl << flush;
      int ret = write(args[2], &data_seg[args[5]], args[3]);
      cout << endl << "=> " << ret;
      break;
  }

  return len;
}

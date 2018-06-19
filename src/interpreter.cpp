#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include "interpreter.hpp"
#include "inst.hpp"
#include "util.hpp"
#include "consts.hpp"
#include "message.hpp"

using namespace std;

function<int(int, int)> Interpreter::fn_mov = [](int d, int s) { return s; };
function<int(int, int)> Interpreter::fn_sub = [](int d, int s) { return d - s; };

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

  cout << " AX   BX   CX   DX   SP   BP   SI   DI   IP" << endl;

  while (pc < header.a_text) {
    cout << util::reg_state_str(reg);
    cout << util::line_number_str(pc);

    const char *head = &text_seg[pc];
    const unsigned short top = head[0] & 0xff;
    const unsigned short dbl = (head[0] << 8 & 0xff00) + (head[1] & 0xff);

    if      ((top & 0b11111100)
                 == 0b10001000) pc += proc_rm_and_reg_to_either(head, "mov", true, fn_mov);
    else if ((top & 0b11110000)
                 == 0b10110000) pc += proc_imm_to_reg(head, "mov", fn_mov);
    else if ((dbl & 0b1111110000111000)
                 == 0b1000000000101000) pc += proc_imm_to_rm(head, "sub", SIGNED, true, fn_sub);
    else if ((top & 0b11111111)
                 == 0b11001101) pc += inst_int_1(head);
    else {
      cout << util::instruction_str(head, 1) << "************";
      pc++;
    }

    cout << endl;
  }
}

size_t Interpreter::proc_rm_and_reg_to_either(const char *head, const char *name, const bool d, function<int(int, int)> fn) {
  Inst inst(head);
  if (d) inst.d = head[0] >> 1;
  inst.w = head[0];
  inst.set_mod_sec();

  const size_t len = inst.get_inst_len();
  cout << inst.get_inst_str(name);
  cout << inst.get_dist_str();

  if (inst.d) {
    const int data = fn(inst.get_reg_value(reg, data_seg), inst.get_rm_value(reg, data_seg));
    inst.put_value_reg(reg, data);
  } else {
    const int data = fn(inst.get_rm_value(reg, data_seg), inst.get_reg_value(reg, data_seg));
    inst.put_value_rm(reg, data_seg, data);
  }

  return len;
}

size_t Interpreter::proc_imm_to_rm(const char *head, const char *name, const bool sign, const bool s, function<int(int, int)> fn) {
  Inst inst(head);
  if (s) inst.s = head[0] >> 1;
  inst.w = head[0];
  inst.set_mod_sec();
  inst.set_data();

  const size_t len = inst.get_inst_len();
  string fixed_name(name);
  fixed_name += (!inst.w && inst.mod != 0b11) ? " byte" : "";
  cout << inst.get_inst_str(fixed_name.c_str());
  cout << inst.get_rm_str() << ", " << inst.get_data_str(sign);

  const int data = fn(inst.get_rm_value(reg, data_seg), inst.get_data_value());
  inst.put_value_rm(reg, data_seg, data);

  return len;
}

size_t Interpreter::proc_imm_to_reg(const char *head, const char *name, function<int(int, int)> fn) {
  Inst inst(head);
  inst.w = head[0] >> 3;
  inst.reg = head[0];
  inst.set_data();

  const size_t len = inst.get_inst_len();
  cout << inst.get_inst_str(name);
  cout << inst.get_reg_name() << ", " << inst.get_data_str();

  const int data = fn(inst.get_reg_value(reg, data_seg), inst.get_data_value());
  inst.put_value_reg(reg, data);

  return len;
}

size_t Interpreter::inst_int_1(const char *head) {
  const unsigned char type = util::get_data_narrow(&head[1]);
  const size_t len = 2;
  cout << util::instruction_str(head, len) << "int ";
  cout << util::data_str_narrow(type);

  Message *m = reinterpret_cast<Message *>(&data_seg[reg.b.x]);

  switch (m->m_type) {
    case 1: // exit
      cout << endl << "<exit(" << m->m1_i1 << ")>";
      cout << endl << flush;
      exit(m->m1_i1);
      break;
    case 4: // write
      cout << endl << "<write(";
      cout << m->m1_i1 << ", ";
      cout << "0x" << util::hex_str(m->m1_p1, sizeof(m->m1_p1) * 2) << ", ";
      cout << m->m1_i2 << ")>";
      cout << endl << flush;
      int ret = write(m->m1_i1, &data_seg[m->m1_p1], m->m1_i2);
      cout << endl << "=> " << ret;
      break;
  }

  return len;
}

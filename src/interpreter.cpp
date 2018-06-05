#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include "interpreter.hpp"
#include "inst.hpp"

using namespace std;

void Interpreter::interpret() {
  ifs.read(reinterpret_cast<char *>(&header), sizeof(header));
  char text[header.a_text];
  ifs.read(text, sizeof(text));
  char data[header.a_data];
  ifs.read(data, sizeof(data));

  execute();
}

void Interpreter::execute() {
  cout << "execute!!" << endl;
}

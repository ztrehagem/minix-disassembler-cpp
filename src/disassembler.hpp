#pragma once

#include <fstream>

using namespace std;

class Disassembler {
public:
  Disassembler(ifstream &ifs);
  void disassemble();

private:
  ifstream& ifs;
};

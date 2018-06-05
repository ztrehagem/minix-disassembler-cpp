#pragma once

#include <fstream>
#include "a.out.hpp"
#include "inst.hpp"

using namespace std;

class Interpreter {
public:
  Interpreter(ifstream &ifs) : ifs(ifs) {};
  void interpret();

private:
  ifstream &ifs;
  struct exec header;

  void execute();
};

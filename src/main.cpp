#include <iostream>
#include "disassembler.hpp"

using namespace std;

int main(int argc, char const *argv[]) {

  ifstream ifs("./a.out");

  if (ifs.fail()) {
    cerr << "'a.out' is not found" << endl;
    return 1;
  }

  Disassembler disassembler(ifs);
  disassembler.disassemble();

  cout << endl;


  return 0;
}

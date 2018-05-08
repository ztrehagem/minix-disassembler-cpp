#include <iostream>
#include <fstream>
#include "disassembler.hpp"

using namespace std;

Disassembler::Disassembler(ifstream &ifs) : ifs(ifs) {

}

void Disassembler::disassemble() {
  cout << "Hello!!" << endl;
}

#include <iostream>
#include "interpreter.hpp"

using namespace std;

int main(int argc, char const *argv[]) {

  if (argc < 2) {
    cerr << "please assign .out (binary) file" << endl;
    return 1;
  }

  const char *filename = argv[1];

  ifstream ifs(filename);

  if (ifs.fail()) {
    cerr << "failed to open '" << filename << "'" << endl;
    return 1;
  }

  Interpreter interpreter(ifs);
  interpreter.interpret();

  cout << endl;


  return 0;
}

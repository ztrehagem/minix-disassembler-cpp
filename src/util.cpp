#include <string>
#include <sstream>
#include <iomanip>
#include "util.hpp"

using namespace std;

unsigned short util::get_data_wide(const char *head) {
  return (head[0] & 0xff) + ((head[1] & 0xff) << 8);
}

unsigned char util::get_data_narrow(const char *head) {
  return head[0] & 0xff;
}

string util::data_str_wide(const unsigned short data, const bool nat, const bool sign) {
  return hex_str(sign ? static_cast<short>(data & 0xffff) : (data & 0xffff), nat ? 0 : 4);
}

string util::data_str_narrow(const unsigned char data, const bool nat, const bool sign) {
  return hex_str(sign ? static_cast<char>(data & 0xff) : (data & 0xff), nat ? 0 : 2);
}

string util::line_number_str(const size_t n) {
  return hex_str(n & 0xffff, 4) + ": ";
}

string util::instruction_str(const char *head, const size_t len) {
  ostringstream ss;
  size_t i = 0;
  for (; i < len; i++) ss << hex_str(head[i] & 0xff, 2);
  for (; i < 7; i++) ss << "  ";
  return ss.str();
}

string util::hex_str(int value, size_t w) {
  ostringstream ss;
  if (value < 0) {
    ss << '-';
    value = -value;
  }
  if (w > 0) {
    ss << setfill('0') << setw(w) << hex << value;
  } else {
    ss << hex << value;
  }
  return ss.str();
}

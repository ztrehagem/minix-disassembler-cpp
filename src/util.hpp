#pragma once

#include <string>

namespace util {
  unsigned short get_data_wide(const char *head);
  unsigned char get_data_narrow(const char *head);
  std::string data_str_wide(const unsigned short data, const bool nat = false, const bool sign = false);
  std::string data_str_narrow(const unsigned char data, const bool nat = false, const bool sign = false);
  std::string line_number_str(const size_t);
  std::string instruction_str(const char *head, const size_t len);
  std::string hex_str(int value, size_t w = 0);
}

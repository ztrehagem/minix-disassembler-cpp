#pragma once

#include <string>

using namespace std;

struct Inst {
  unsigned s:1;
  unsigned v:1;
  unsigned w:1;
  unsigned d:1;
  unsigned reg:3;
  unsigned mod:2;
  unsigned rm:3;
  union {
    unsigned wide:16;
    unsigned narrow:8;
  } data;
  signed disp:16;
  size_t disp_size;
  size_t data_size;
  bool has_mod_sec;
  bool has_data_sec;

  Inst() {
    s = 0;
    v = 0;
    w = 1;
    d = 0;
    reg = 0;
    mod = 0;
    rm = 0;
    disp = 0;
    disp_size = 0;
    data_size = 0;
    has_mod_sec = false;
    has_data_sec = false;
  }

  void set_mod_sec(const char *head);
  void set_data(const char *head);
  bool is_wide_data();
  size_t get_inst_len();
  string get_reg_name(const bool is_rm = false);
  string get_rm_str();
  string get_data_str(const bool as_natural = false);
  string get_dist_str();
};

#ifndef JPEG
#define JPEG

#include <array>
#include <vector>
#include <iostream>

class JPEGImage {
private:
  typedef struct{
    int left;
    int right;
    int value;
  } node;

  unsigned char sof_precision;
  int height;
  int width;
  unsigned char y_qt_id, cr_qt_id, cb_qt_id;
  std::array<std::array<int, 64>, 4> qts;
  std::array<std::array<int, 16>, 4> hts_digits;
  std::array<std::vector<int>, 4> hts_codewords;
  std::array<std::vector<node>, 4> hts;

  int convert_ht_id(int);
  int grow_ht(std::vector<node> &, int, bool);
  void grow_ht(std::vector<node> &, int, bool, int);

public:
  void set_sof_precision(unsigned char);
  unsigned char get_sof_precision();
  void set_height(int);
  int get_height();
  void set_width(int);
  int get_width();
  void set_color_factor(unsigned char, unsigned char, unsigned char);
  void set_qts(int, std::array<int, 64> &);
  void set_hts(int, std::array<int, 16> &, std::vector<int> &);
  void create_hts();
};

#endif

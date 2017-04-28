#ifndef JPEG
#define JPEG

#include <iostream>
#include <array>
#include <vector>
#include <bitset>

class JPEGImage {
private:
  typedef struct {
    int left;
    int right;
    int value;
  } node;

  typedef struct {
    unsigned int codeword;
    int value;
    int length;
  } mask;

  unsigned char sof_precision;
  int height;
  int width;
  unsigned char y_qt_id, cr_qt_id, cb_qt_id;
  unsigned char y_ht_id, cr_ht_id, cb_ht_id;
  unsigned char y_s, cr_s, cb_s;
  std::array<std::array<int, 64>, 4> qts;
  std::array<std::array<int, 16>, 4> hts_digits;
  std::array<std::vector<int>, 4> hts_codewords;
  std::array<std::vector<node>, 4> hts;
  std::array<std::vector<mask>, 4> hts_mask;
  std::vector<unsigned char> data;

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
  void set_color_ht_id(unsigned char, unsigned char);
  void set_qts(int, std::array<int, 64> &);
  void set_hts(int, std::array<int, 16> &, std::vector<int> &);
  void create_hts();
  void set_data(std::vector<unsigned char> &);
};

#endif

#ifndef JPEG
#define JPEG

#include <iostream>
#include <array>
#include <vector>
#include <bitset>
#include <cmath>

class JPEGImage {
private:
  struct codeword {
    unsigned int codeword;
    int value;
    int length;
  };

  struct mcu {
    int height;
    int width;
    std::vector<std::array<int, 64>> y;
    std::vector<std::array<int, 64>> cr;
    std::vector<std::array<int, 64>> cb;
  };

  unsigned char sof_precision;
  int height;
  int width;
  int y_qt_id, cr_qt_id, cb_qt_id;
  int y_dc_ht_id, cr_dc_ht_id, cb_dc_ht_id;
  int y_ac_ht_id, cr_ac_ht_id, cb_ac_ht_id;
  unsigned char y_s_v, cr_s_v, cb_s_v;
  unsigned char y_s_h, cr_s_h, cb_s_h;
  unsigned char v_max, h_max;
  std::array<std::array<int, 64>, 4> qts;
  std::array<std::array<int, 16>, 4> hts_digits;
  std::array<std::vector<int>, 4> hts_codewords;
  std::array<std::vector<codeword>, 4> hts;
  std::array<unsigned int, 16> mask;
  std::vector<mcu> mcus;
  std::vector<unsigned char> data;
  unsigned int now;
  int now_length;
  unsigned int buffer;
  int buffer_length;

  int convert_ht_id(int);
  void create_hts();
  void decode_data();
  std::array<int, 64> build_block(int &, int, int);
  int convert_to_real_value(int, int);
  codeword ht_process(int &, int);
  void align(int &);
  unsigned int ask_buffer_bits(int &, int);
  unsigned int ask_now_bits(int &, int);
  void dc_diff_decode();
  void dequantize();
  void inverse_zigzag();
  void zigzag_process(std::array<int, 64> &, std::array<int, 64> &);
  void inverse_dct();
  void idct_process(std::array<int, 64> &);
  int idct(std::array<int, 64> &, int x, int y);

public:
  JPEGImage();
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
  void set_data(std::vector<unsigned char> &);
  void decode();
};

#endif

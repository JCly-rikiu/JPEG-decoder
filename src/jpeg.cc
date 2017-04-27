#include "jpeg.h"

void JPEGImage::set_sof_precision(unsigned char sof_precision) {
  this->sof_precision = sof_precision;
}

unsigned char JPEGImage::get_sof_precision() {
  return this->sof_precision;
}

void JPEGImage::set_height(int height) {
  this->height = height;
}

int JPEGImage::get_height() {
  return this->height;
}

void JPEGImage::set_width(int width) {
  this->width = width;
}

int JPEGImage::get_width() {
  return this->width;
}

void JPEGImage::set_color_factor(unsigned char color_id, unsigned char sample_factor, unsigned char qt_id) {
  switch(color_id) {
   case 0: this->y_qt_id = qt_id; break;
   case 1: this->cr_qt_id = qt_id; break;
   case 2: this->cb_qt_id = qt_id; break;
  }
}

void JPEGImage::set_qts(int qt_id, std::array<int, 64> &qt) {
  this->qts[qt_id] = qt;
}

void JPEGImage::set_hts(int ht_id, std::array<int, 16> &digits, std::vector<int> &codewords) {
  ht_id = JPEGImage::convert_ht_id(ht_id);
  this->hts_digits[ht_id] = digits;
  this->hts_codewords[ht_id] = codewords;
}

int JPEGImage::convert_ht_id(int ht_id) {
  if (ht_id == 0x10)
    return 2;
  if (ht_id == 0x11)
    return 3;
  return ht_id;
}

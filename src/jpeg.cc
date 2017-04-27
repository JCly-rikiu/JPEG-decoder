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

void JPEGImage::create_hts() {
  for (int ht_id = 0; ht_id != 4; ht_id++) {
    std::vector<node> ht;

    node head = {-1, -1, -1};
    ht.push_back(head);

    int count = 0;
    int code = 0;
    for (int digit = 0; digit != 16; digit++) {
      if (hts_digits[ht_id][digit] == 0) {
        code <<= 1;
        continue;
      }

      for (int i = 0; i != hts_digits[ht_id][digit]; i++) {
        std::bitset<16> code_bits(code);
        int now = 0;
        for (int j = digit; j >= 0; j--) {
          if (j == 0)
            grow_ht(ht, now, code_bits[j], hts_codewords[ht_id][count]);
          else
            now = grow_ht(ht, now, code_bits[j]);
        }

        count++;
        code++;
      }
      code <<= 1;
    }

    this->hts[ht_id] = ht;
  }

  std::cerr << "HTs created." << std::endl;
}

int JPEGImage::grow_ht(std::vector<node> &ht, int now, bool bit) {
  if (bit) {
    if (ht[now].right == -1) {
      node new_node = {-1, -1, -1};
      ht.push_back(new_node);
      ht[now].right = ht.size() - 1;
      return ht[now].right;
    } else {
      return ht[now].right;
    }
  } else {
    if (ht[now].left == -1) {
      node new_node = {-1, -1, -1};
      ht.push_back(new_node);
      ht[now].left = ht.size() - 1;
      return ht[now].left;
    } else {
      return ht[now].left;
    }
  }
}

void JPEGImage::grow_ht(std::vector<node> &ht, int now, bool bit, int value) {
  if (bit) {
    if (ht[now].right == -1) {
      node new_node = {-1, -1, value};
      ht.push_back(new_node);
      ht[now].right = ht.size() - 1;
    }
  } else {
    if (ht[now].left == -1) {
      node new_node = {-1, -1, value};
      ht.push_back(new_node);
      ht[now].left = ht.size() - 1;
    }
  }
}

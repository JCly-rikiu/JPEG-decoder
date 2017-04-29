#include "jpeg.h"

JPEGImage::JPEGImage() {
  this->v_max = 0;
  this->h_max = 0;

  this->data_pos = 0;
  this->now = 0;
  this->now_length = 0;
  this->buffer = 0;
  this->buffer_length = 0;
}

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
   case 1:
    this->y_qt_id = static_cast<int>(qt_id);
    this->y_s_v = sample_factor >> 4;
    this->y_s_h = sample_factor & 0x0f;
    if (this->y_s_v > this->v_max) this->v_max = this->y_s_v;
    if (this->y_s_h > this->h_max) this->h_max = this->y_s_h;
    break;
   case 2:
    this->cr_qt_id = static_cast<int>(qt_id);
    this->cr_s_v = sample_factor >> 4;
    this->cr_s_h = sample_factor & 0x0f;
    if (this->cr_s_v > this->v_max) this->v_max = this->cr_s_v;
    if (this->cr_s_h > this->h_max) this->h_max = this->cr_s_h;
    break;
   case 3:
    this->cb_qt_id = static_cast<int>(qt_id);
    this->cb_s_v = sample_factor >> 4;
    this->cb_s_h = sample_factor & 0x0f;
    if (this->cb_s_v > this->v_max) this->v_max = this->cb_s_v;
    if (this->cb_s_h > this->h_max) this->h_max = this->cb_s_h;
    break;
  }
}

void JPEGImage::set_color_ht_id(unsigned char color_id, unsigned char ht_id) {
  int dc_ht_id = static_cast<int>(ht_id >> 4);
  if (dc_ht_id == 0)
    dc_ht_id = 0;
  else
    dc_ht_id = 1;
  int ac_ht_id = static_cast<int>(ht_id & 0x0f);
    if (ac_ht_id == 0)
      ac_ht_id = 2;
    else
      ac_ht_id = 3;

  switch(color_id) {
   case 1:
    this->y_dc_ht_id = dc_ht_id;
    this->y_ac_ht_id = ac_ht_id;
    break;
   case 2:
    this->cr_dc_ht_id = dc_ht_id;
    this->cr_ac_ht_id = ac_ht_id;
    break;
   case 3:
    this->cb_dc_ht_id = dc_ht_id;
    this->cb_ac_ht_id = ac_ht_id;
    break;
  }
}

void JPEGImage::set_qts(int qt_id, std::array<int, 64> &qt) {
  this->qts[qt_id] = qt;
}

void JPEGImage::set_hts(int ht_id, std::array<int, 16> &digits, std::vector<int> &codewords) {
  ht_id = convert_ht_id(ht_id);
  this->hts_digits[ht_id] = digits;
  this->hts_codewords[ht_id] = codewords;
}

void JPEGImage::set_data(std::vector<unsigned char> &data) {
  this->data = data;
}

void JPEGImage::decode() {
  std::cerr << "Start decoding..." << std::endl;
  create_hts();
  decode_data();
  dc_diff_decode();
  dequantize();
  inverse_zigzag();
  inverse_dct();
  to_rgb_image();
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
    std::vector<codeword> ht;

    int count = 0;
    unsigned int code = 0;
    for (int digit = 0; digit != 16; digit++) {
      if (hts_digits[ht_id][digit] == 0) {
        code <<= 1;
        continue;
      }

      for (int i = 0; i != hts_digits[ht_id][digit]; i++) {
        std::bitset<16> code_bits(code);

        codeword new_codeword = { code << (16 - digit - 1), hts_codewords[ht_id][count], digit + 1 };
        ht.push_back(new_codeword);

        count++;
        code++;
      }
      code <<= 1;
    }

    this->hts[ht_id] = ht;
  }

  unsigned int temp = 1;
  for (int i = 0; i != 16; i++) {
    this->mask[i] = (temp << (15 - i));
    temp <<= 1;
    temp++;
  }

  std::cerr << "HTs created." << std::endl;
}

void JPEGImage::decode_data() {
  this->mcu_height = static_cast<int>(this->v_max) * 8;
  this->mcu_width = static_cast<int>(this->h_max) * 8;
  std::cerr << "mcu height pixel: " << this->mcu_height << " mcu width pixel: " << this->mcu_width << std::endl;

  this->mcu_h_size = this->height / this->mcu_height;
  if (this->height % this->mcu_height != 0) this->mcu_h_size++;
  this->mcu_w_size = this->width / this->mcu_width;
  if (this->width % this->mcu_width != 0) this->mcu_w_size++;
  std::cerr << "mcu height num: " << this->mcu_h_size << " mcu width num: " << this->mcu_w_size << std::endl;

  for (int mcu_i = 0; mcu_i != this->mcu_h_size * this->mcu_w_size; mcu_i++) {
    mcu m;
    for (int y_i = 0; y_i != this->y_s_v * this->y_s_h; y_i++) {
      m.y.push_back(build_block(this->y_dc_ht_id, this->y_ac_ht_id));
    }
    for (int cr_i = 0; cr_i != this->cr_s_v * this->cr_s_h; cr_i++) {
      m.cr.push_back(build_block(this->cr_dc_ht_id, this->cr_ac_ht_id));
    }
    for (int cb_i = 0; cb_i != this->cb_s_v * this->cb_s_h; cb_i++) {
      m.cb.push_back(build_block(this->cb_dc_ht_id, this->cb_ac_ht_id));
    }

    this->mcus.push_back(m);
  }

  std::cerr << "data pos: " << this->data_pos << std::endl;
}

std::array<int, 64> JPEGImage::build_block(int dc_ht_id, int ac_ht_id) {
  std::array<int, 64> block = {};

  int count = 0;
  codeword c = ht_process(dc_ht_id);
  block[count++]  = convert_to_real_value(c.value, ask_now_bits(c.value));

  while (count < 64) {
    c = ht_process(ac_ht_id);
    if (c.value == 0)
      break;
    int run = c.value >> 4;
    for (int i = 0; i != run; i++)
      block[count++] = 0;
    block[count++] = convert_to_real_value(c.value & 0x0f, ask_now_bits(c.value & 0x0f));
  }

  return block;
}

int JPEGImage::convert_to_real_value(int length, int pos) {
  if (length == 0)
    return 0;
  int temp = 1 << (length - 1);
  int diff = pos - temp;
  if (diff >= 0)
    return temp + diff;
  else
    return -temp + diff + 1;
}

JPEGImage::codeword JPEGImage::ht_process(int ht_id) {
  align();
  for (codeword c : this->hts[ht_id]) {
    if (c.codeword == (this->mask[c.length - 1] & this->now)) {
      ask_now_bits(c.length);
      return c;
    }
  }

  return codeword { 0, 0, -1 };
}

void JPEGImage::align() {
  this->now |= ask_buffer_bits(16 - this->now_length);
  this->now_length = 16;
}

unsigned int JPEGImage::ask_buffer_bits(int bits) {
  if (bits == 0)
    return 0;

  while (this->buffer_length <= 16 && this->data_pos < this->data.size()) {
    this->buffer >>= 24 - this->buffer_length;
    this->buffer |= static_cast<unsigned int>(this->data[this->data_pos++]);
    this->buffer <<= 24 - this->buffer_length;
    this->buffer_length += 8;
  }

  unsigned int temp = this->mask[bits - 1] & (this->buffer >> 16);
  temp >>= 16 - bits;
  this->buffer <<= bits;
  this->buffer_length -= bits;

  return temp;
}

unsigned int JPEGImage::ask_now_bits(int bits) {
  if (bits == 0)
    return 0;

  align();

  unsigned int temp = this->mask[bits - 1] & this->now;
  temp >>= 16 - bits;
  this->now <<= bits;
  this->now_length -= bits;

  return temp;
}

void JPEGImage::dc_diff_decode() {
  int last_y = 0;
  int last_cr = 0;
  int last_cb = 0;

  for (auto &m : this->mcus) {
    for (auto &block : m.y) {
      block[0] += last_y;
      last_y = block[0];
    }
    for (auto &block : m.cr) {
      block[0] += last_cr;
      last_cr = block[0];
    }
    for (auto &block : m.cb) {
      block[0] += last_cb;
      last_cb = block[0];
    }
  }
}

void JPEGImage::dequantize() {
  auto y_qt = this->qts[this->y_qt_id];
  auto cr_qt = this->qts[this->cr_qt_id];
  auto cb_qt = this->qts[this->cb_qt_id];

  for (auto &m : this->mcus) {
    for (auto &block : m.y)
      std::transform(block.begin(), block.end(), y_qt.begin(), block.begin(), std::multiplies<int>());
    for (auto &block : m.cr)
      std::transform(block.begin(), block.end(), cr_qt.begin(), block.begin(), std::multiplies<int>());
    for (auto &block : m.cb)
      std::transform(block.begin(), block.end(), cb_qt.begin(), block.begin(), std::multiplies<int>());
  }
}

void JPEGImage::inverse_zigzag() {
  std::array<int, 64> table;
  int count = 0;
  for (int sum = 0; sum != 8; sum++) {
    if ((sum & 1) == 1)
      for (int i = 0; i != sum + 1; i++)
        table[count++] = i * 8 + sum - i;
    else
      for (int i = sum; i >= 0; i--)
        table[count++] = i * 8 + sum - i;
  }
  for (int sum = 8; sum != 15; sum++)  {
    if ((sum & 1) == 1)
      for (int i = sum - 7; i != 8; i++)
        table[count++] = i * 8 + sum - i;
    else
      for (int i = 7; i >= sum - 7; i--)
        table[count++] = i * 8 + sum - i;
  }


  for (auto &m : this->mcus) {
    for (auto &block : m.y)
      zigzag_process(block, table);
    for (auto &block : m.cr)
      zigzag_process(block, table);
    for (auto &block : m.cb)
      zigzag_process(block, table);
  }
}

void JPEGImage::zigzag_process(std::array<int, 64> &block, std::array<int, 64> &table) {
  auto temp = block;
  for (int i = 0; i != 64; i++)
    block[table[i]] = temp[i];
}

void JPEGImage::inverse_dct() {
  for (auto &m : this->mcus) {
    for (auto &block : m.y)
      idct_process(block);
    for (auto &block : m.cr)
      idct_process(block);
    for (auto &block : m.cb)
      idct_process(block);
  }
}

void JPEGImage::idct_process(std::array<int, 64> &block) {
  auto temp = block;
  for (int i = 0; i != 8; i++)
    for (int j = 0; j != 8; j++)
      block[i * 8 + j] = idct(temp, i, j);
}

int JPEGImage::idct(std::array<int, 64> &block, int x, int y) {
  double sum = 0.0;
  const double pi = std::acos(-1);
  for (int u = 0; u != 8; u++) {
    for (int v = 0; v != 8; v++) {
      double c = 1.0;
      if (u == 0) c *= (1 / std::sqrt(2));
      if (v == 0) c *= (1 / std::sqrt(2));

      sum += c * block[u * 8 + v] * std::cos((2 * x + 1) * u * pi / 16) * std::cos((2 * y + 1) * v * pi / 16);
    }
  }

  return static_cast<int>(std::round(sum / 4)) + 128;
}

void JPEGImage::to_rgb_image() {
  this->image = std::vector<std::vector<int>>(this->height);
  std::for_each(this->image.begin(), this->image.end(), [&](std::vector<int> &v) { v = std::vector<int>(this->width); } );

  for (int mcu_i = 0; mcu_i != this->mcu_h_size; mcu_i++) {
    for (int mcu_j = 0; mcu_j != this->mcu_w_size; mcu_j++) {
      mcu m = this->mcus[mcu_i * this->mcu_w_size + mcu_j];
      for (int i = 0; i != this->mcu_height; i++) {
        for (int j = 0; j != this->mcu_width; j++) {
          int x = mcu_i * this->mcu_height + i;
          int y = mcu_j * this->mcu_width + j;

          if (x >= this->height || y >= this->width)
            continue;

          int y_i = i / (this->v_max / this->y_s_v);
          int y_j = j / (this->h_max / this->y_s_h);
          double color_y = m.y[(y_i / 8) * this->y_s_h + (y_j / 8)][(y_i % 8) * 8 + (y_j % 8)];

          int cr_i = i / (this->v_max / this->cr_s_v);
          int cr_j = j / (this->h_max / this->cr_s_h);
          double color_cr = m.cr[(cr_i / 8) * this->cr_s_h + (cr_j / 8)][(cr_i % 8) * 8 + (cr_j % 8)];

          int cb_i = i / (this->v_max / this->cb_s_v);
          int cb_j = j / (this->h_max / this->cb_s_h);
          double color_cb = m.cb[(cb_i / 8) * this->cb_s_h + (cb_j / 8)][(cb_i % 8) * 8 + (cb_j % 8)];

          int r = static_cast<int>(std::round(color_y + 1.402 * (color_cr - 128)));
          int g = static_cast<int>(std::round(color_y - 0.344136 * (color_cb - 128) - 0.714136 * (color_cr - 128)));
          int b = static_cast<int>(std::round(color_y + 1.772 * (color_cb - 128)));

          check_rgb_valid(r);
          check_rgb_valid(g);
          check_rgb_valid(b);

          this->image[x][y] = (r << 16) | (g << 8) | b;
        }
      }
    }
  }
}

void JPEGImage::check_rgb_valid(int &v) {
  if (v < 0)
    v = 0;
  else if (v > 255)
    v = 255;
}

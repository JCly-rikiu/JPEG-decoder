#include "jpeg.h"

JPEGImage::JPEGImage() {
  this->v_max = 0;
  this->h_max = 0;

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
  int mcu_height = static_cast<int>(this->v_max) * 8;
  int mcu_width = static_cast<int>(this->h_max) * 8;
  std::cerr << "mcu height pixel: " << mcu_height << " mcu width pixel: " << mcu_width << std::endl;

  int mcu_h_size = this->height / mcu_height;
  if (this->height % mcu_height != 0) mcu_h_size++;
  int mcu_w_size = this->width / mcu_width;
  if (this->width % mcu_width != 0) mcu_w_size++;
  std::cerr << "mcu height num: " << mcu_h_size << " mcu width num: " << mcu_w_size << std::endl;

  int data_pos = 0;
  for (int mcu_i = 0; mcu_i != mcu_h_size * mcu_w_size; mcu_i++) {
    mcu m;
    m.height = mcu_height;
    m.width = mcu_width;

    for (int y_i = 0; y_i != this->y_s_v * this->y_s_h; y_i++) {
      m.y.push_back(build_block(data_pos, this->y_dc_ht_id, this->y_ac_ht_id));
    }
    for (int cr_i = 0; cr_i != this->cr_s_v * this->cr_s_h; cr_i++) {
      m.cr.push_back(build_block(data_pos, this->cr_dc_ht_id, this->cr_ac_ht_id));
    }
    for (int cb_i = 0; cb_i != this->cb_s_v * this->cb_s_h; cb_i++) {
      m.cb.push_back(build_block(data_pos, this->cb_dc_ht_id, this->cb_ac_ht_id));
    }

    this->mcus.push_back(m);
  }

  std::cerr << "data pos: " << data_pos << std::endl;
}

std::array<int, 64> JPEGImage::build_block(int &data_pos, int dc_ht_id, int ac_ht_id) {
  std::array<int, 64> block = {};

  int count = 0;
  codeword c = ht_process(data_pos, dc_ht_id);
  block[count++]  = convert_to_real_value(c.value, ask_now_bits(data_pos, c.value));

  while (count < 64) {
    c = ht_process(data_pos, ac_ht_id);
    if (c.value == 0)
      break;
    int run = c.value >> 4;
    for (int i = 0; i != run; i++)
      block[count++] = 0;
    block[count++] = convert_to_real_value(c.value & 0x0f, ask_now_bits(data_pos, c.value & 0x0f));
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

JPEGImage::codeword JPEGImage::ht_process(int &data_pos, int ht_id) {
  align(data_pos);
  for (codeword c : this->hts[ht_id]) {
    if (c.codeword == (this->mask[c.length - 1] & this->now)) {
      ask_now_bits(data_pos, c.length);
      return c;
    }
  }

  return codeword { 0, 0, -1 };
}

void JPEGImage::align(int &data_pos) {
  this->now |= ask_buffer_bits(data_pos, 16 - this->now_length);
  this->now_length = 16;
}

unsigned int JPEGImage::ask_buffer_bits(int &data_pos, int bits) {
  if (bits == 0)
    return 0;

  while (this->buffer_length <= 16 && data_pos < this->data.size()) {
    this->buffer >>= 24 - this->buffer_length;
    this->buffer |= static_cast<unsigned int>(this->data[data_pos++]);
    this->buffer <<= 24 - this->buffer_length;
    this->buffer_length += 8;
  }

  unsigned int temp = this->mask[bits - 1] & (this->buffer >> 16);
  temp >>= 16 - bits;
  this->buffer <<= bits;
  this->buffer_length -= bits;

  return temp;
}

unsigned int JPEGImage::ask_now_bits(int &data_pos, int bits) {
  if (bits == 0)
    return 0;

  align(data_pos);

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

  return static_cast<int>(std::round(sum / 4));
}

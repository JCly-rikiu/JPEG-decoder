#include "util.h"

void parse(JPEGImage &image, std::string &filename) {
  std::ifstream file(filename, std::ios::in | std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Can not open jpeg file \"" << filename << "\"" << std::endl;
    exit(EXIT_FAILURE);
  }

  unsigned char tag;
  while (!file.eof()) {
    file.read(reinterpret_cast<char*>(&tag), 1);

    int length;
    if (tag == Tag::START) {
      file.read(reinterpret_cast<char*>(&tag), 1);
      switch (tag) {
       case Tag::SOI:
        std::cout << "SOI" << std::endl;
        break;
       case Tag::APP0:
        length = get_2bytes(file) - 2;
        std::cout << "APP0: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::APP1:
        length = get_2bytes(file) - 2;
        std::cout << "APP1: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::APP2:
        length = get_2bytes(file) - 2;
        std::cout << "APP2: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::APP3:
        length = get_2bytes(file) - 2;
        std::cout << "APP3: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::APP4:
        length = get_2bytes(file) - 2;
        std::cout << "APP4: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::APP5:
        length = get_2bytes(file) - 2;
        std::cout << "APP5: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::APP6:
        length = get_2bytes(file) - 2;
        std::cout << "APP6: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::APP7:
        length = get_2bytes(file) - 2;
        std::cout << "APP7: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::APP8:
        length = get_2bytes(file) - 2;
        std::cout << "APP8: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::APP9:
        length = get_2bytes(file) - 2;
        std::cout << "APP9: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::APP10:
        length = get_2bytes(file) - 2;
        std::cout << "APP10: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::APP11:
        length = get_2bytes(file) - 2;
        std::cout << "APP11: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::APP12:
        length = get_2bytes(file) - 2;
        std::cout << "APP12: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::APP13:
        length = get_2bytes(file) - 2;
        std::cout << "APP13: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::APP14:
        length = get_2bytes(file) - 2;
        std::cout << "APP14: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::APP15:
        length = get_2bytes(file) - 2;
        std::cout << "APP15: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::DQT:
        length = get_2bytes(file) - 2;
        std::cout << "DQT: " << length << std::endl;
        parse_dqt(image, file, length);
        break;
       case Tag::SOF0:
        length = get_2bytes(file) - 2;
        std::cout << "SOF0: " << length << std::endl;
        parse_sof0(image, file, length);
        break;
       case Tag::DHT:
        length = get_2bytes(file) - 2;
        std::cout << "DHT: " << length << std::endl;
        parse_dht(image, file, length);
        break;
       case Tag::DRI:
        length = get_2bytes(file) - 2;
        std::cout << "DRI: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::SOS:
        length = get_2bytes(file) - 2;
        std::cout << "SOS: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::EOI:
        std::cout << "EOI" << std::endl;
        break;
      }
    }
  }

  file.close();
}

unsigned char get_byte(std::ifstream &file) {
  unsigned char value;
  file.read(reinterpret_cast<char*>(&value), 1);

  return value;
}

int get_2bytes(std::ifstream &file) {
  unsigned char hi = 0, lo = 0;
  file.read(reinterpret_cast<char*>(&hi), 1);
  file.read(reinterpret_cast<char*>(&lo), 1);

  return static_cast<int>((hi << 8) | lo);
}

void parse_dqt(JPEGImage &image, std::ifstream &file, int length) {
  while (length > 0) {
    int value = static_cast<int>(get_byte(file));
    int precision = static_cast<int>(value >> 4);

    std::array<int, 64> qt;
    for (int i = 0; i != 64; i++) {
      if (precision == 0)
        qt[i] = static_cast<int>(get_byte(file));
      else
        qt[i] = get_2bytes(file);
    }

    image.set_qts(static_cast<int>(value & 0x0f), qt);

    length -= 64 * (precision + 1) + 1;
  }
}

void parse_sof0(JPEGImage &image, std::ifstream &file, int length) {
  image.set_sof_precision(get_byte(file));
  image.set_height(get_2bytes(file));
  image.set_width(get_2bytes(file));

  // skip 1:gray scale, 3:YCrCb, 4:CMYK
  file.seekg(1, file.cur);

  for (int i = 0; i != 3; i++)
    image.set_color_factor(get_byte(file), get_byte(file), get_byte(file));
}

void parse_dht(JPEGImage &image, std::ifstream &file, int length) {
  while (length > 0) {
    int type = static_cast<int>(get_byte(file));

    std::array<int, 16> digits;
    int total = 0;
    for (int i = 0; i != 16; i++) {
      digits[i] = static_cast<int>(get_byte(file));
      total += digits[i];
    }

    std::vector<int> codewords;
    for (int i = 0; i != total; i++)
      codewords.push_back(static_cast<int>(get_byte(file)));

    image.set_hts(type, digits, codewords);

    length -= 16 + total + 1;
  }
}

#include "util.h"

void parse(JPEGImage &image, std::string &filename) {
  // open file
  std::ifstream file(filename, std::ios::in | std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Can not open jpeg file \"" << filename << "\"" << std::endl;
    exit(EXIT_FAILURE);
  }

  bool image_data = false;
  std::vector<unsigned char> data;
  while (!file.eof()) {
    unsigned char tag = 0;
    file.read(reinterpret_cast<char*>(&tag), 1);

    if (tag == Tag::FF) {
      int length;
      file.read(reinterpret_cast<char*>(&tag), 1);
      switch (tag) {
       case Tag::SOI:
        std::cerr << "SOI" << std::endl;
        break;
       case Tag::DQT:
        length = get_2bytes(file) - 2;
        std::cerr << "DQT: " << length << std::endl;
        parse_dqt(image, file, length);
        break;
       case Tag::SOF0:
        length = get_2bytes(file) - 2;
        std::cerr << "SOF0: " << length << std::endl;
        parse_sof0(image, file, length);
        break;
       case Tag::DHT:
        length = get_2bytes(file) - 2;
        std::cerr << "DHT: " << length << std::endl;
        parse_dht(image, file, length);
        break;
       case Tag::DRI:
        length = get_2bytes(file) - 2;
        std::cerr << "DRI: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::SOS:
        length = get_2bytes(file) - 2;
        std::cerr << "SOS: " << length << std::endl;
        parse_sos(image, file, length);
        image_data = true;
        break;
       case Tag::EOI:
        image_data = false;
        std::cerr << "Data size: " << data.size() << std::endl;
        image.set_data(data);
        std::cerr << "EOI" << std::endl;
        break;
       case Tag::DATA:
        if (image_data)
          data.push_back(Tag::FF);
        break;
       default:
        if (tag >= Tag::APP0 && tag <= Tag::APP15) {
          length = get_2bytes(file) - 2;
          std::cerr << "APP" << static_cast<int>(tag - Tag::APP0) << ": " << length << std::endl;
          file.seekg(length, file.cur);
          break;
        }

        // if (tag >= Tag::RST0 && tag <= Tag::RST7) {
        // }

        if (image_data)
          data.push_back(tag);
      }
    } else {
      if (image_data)
        data.push_back(tag);
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
    // 1byte (percision/qt_id)
    int value = static_cast<int>(get_byte(file));
    int precision = static_cast<int>(value >> 4);
    int qt_id = static_cast<int>(value & 0x0f);

    // 64 * (precision + 1) bytes (qt)
    std::array<int, 64> qt;
    for (int i = 0; i != 64; i++) {
      if (precision == 0)
        qt[i] = static_cast<int>(get_byte(file));
      else
        qt[i] = get_2bytes(file);
    }
    image.set_qts(qt_id, qt);

    // maybe N qts
    length -= 64 * (precision + 1) + 1;
  }
}

void parse_sof0(JPEGImage &image, std::ifstream &file, int length) {
  // 1byte (image precision)
  image.set_sof_precision(get_byte(file));
  // 4bytes (image height/image width)
  image.set_height(static_cast<int>(get_2bytes(file)));
  image.set_width(static_cast<int>(get_2bytes(file)));

  // skip 1byte (1:gray scale, 3:YCrCb, 4:CMYK)
  file.seekg(1, file.cur);

  // (Y Cr Cb) each 3bytes (color_id, sample_factor, qt_id)
  for (int i = 0; i != 3; i++)
    image.set_color_factor(get_byte(file), get_byte(file), get_byte(file));
}

void parse_dht(JPEGImage &image, std::ifstream &file, int length) {
  while (length > 0) {
    // 1byte (ht_id)
    int type = static_cast<int>(get_byte(file));

    // 16bytes (digit numbers)
    std::array<int, 16> digits;
    int total = 0;
    for (int i = 0; i != 16; i++) {
      digits[i] = static_cast<int>(get_byte(file));
      total += digits[i];
    }

    // total * bytes (codewords)
    std::vector<int> codewords;
    for (int i = 0; i != total; i++)
      codewords.push_back(static_cast<int>(get_byte(file)));

    image.set_hts(type, digits, codewords);

    // maybe N hts
    length -= 16 + total + 1;
  }
}

void parse_sos(JPEGImage &image, std::ifstream &file, int length) {
  // skip 1byte (1:gray scale, 3:YCrCb, 4:CMYK)
  file.seekg(1, file.cur);

  // (Y Cr Cb) each 2bytes (color_id, DC_ht_id/AC_ht_id)
  for (int i = 0; i != 3; i++)
    image.set_color_ht_id(get_byte(file), get_byte(file));

  // skip 3bytes (0x00 0x3f 0x00)
  file.seekg(3, file.cur);
}

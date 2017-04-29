#include "util.h"

void parse(JPEGImage &image, const std::string &filename) {
  // open file
  std::ifstream file(filename, std::ios::in | std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Can not open jpeg file \"" << filename << "\"" << std::endl;
    exit(EXIT_FAILURE);
  }

  bool image_data = false;
  std::vector<unsigned char> data;
  while (true) {
    unsigned char tag = 0;
    tag = get_byte(file);
    if (file.eof())
      break;

    if (tag == Tag::FF) {
      int length;
      tag = get_byte(file);
      switch (tag) {
       case Tag::SOI:
        std::cerr << "SOI" << std::endl;
        break;
       case Tag::DQT:
        length = get_2bytes(file) - 2;
        std::cerr << "DQT" << std::endl;
        std::cerr << "  length: " << length << std::endl;
        parse_dqt(image, file, length);
        break;
       case Tag::SOF0:
        length = get_2bytes(file) - 2;
        std::cerr << "SOF0" << std::endl;
        std::cerr << "  length: " << length << std::endl;
        parse_sof0(image, file, length);
        break;
       case Tag::DHT:
        length = get_2bytes(file) - 2;
        std::cerr << "DHT" << std::endl;
        std::cerr << "  length: " << length << std::endl;
        parse_dht(image, file, length);
        break;
       case Tag::DRI:
        length = get_2bytes(file) - 2;
        std::cerr << "DRI" << std::endl;
        std::cerr << "  length: " << length << std::endl;
        parse_dri(image, file, length);
        break;
       case Tag::SOS:
        length = get_2bytes(file) - 2;
        std::cerr << "SOS" << std::endl;
        std::cerr << "  length: " << length << std::endl;
        parse_sos(image, file, length);
        image_data = true;
        break;
       case Tag::EOI:
        image_data = false;
        std::cerr << "  data size: " << data.size() << std::endl;
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
          std::cerr << "APP" << static_cast<int>(tag - Tag::APP0) << std::endl;
          std::cerr << "  length: " << length << std::endl;
          file.seekg(length, file.cur);
          break;
        }

        if (tag >= Tag::RST0 && tag <= Tag::RST7) {
          break;
        }

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
  char value;
  file.read(&value, 1);
  return static_cast<unsigned char>(value);
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
  unsigned char precision = get_byte(file);
  image.set_sof_precision(precision);
  // 4bytes (image height/image width)
  int height = get_2bytes(file);
  int width = get_2bytes(file);
  image.set_height(height);
  image.set_width(width);

  // skip 1byte (1:gray scale, 3:YCrCb, 4:CMYK)
  unsigned char color_type = get_byte(file);

  std::cerr << "  precision: " << static_cast<int>(precision) << std::endl;
  std::cerr << "  height: " << height << std::endl;
  std::cerr << "  width: " << width << std::endl;
  std::cerr << "  color type: " << static_cast<int>(color_type) << std::endl;

  // (Y Cr Cb) each 3bytes (color_id, sample_factor, qt_id)
  for (int i = 0; i != 3; i++) {
    unsigned char color_id = get_byte(file);
    unsigned char sample_factor = get_byte(file);
    unsigned char qt_id = get_byte(file);
    image.set_color_factor(color_id, sample_factor, qt_id);

    std::cerr << "  " << static_cast<int>(color_id) << " " << static_cast<int>(sample_factor >> 4) << " " << static_cast<int>(sample_factor & 0x0f) << " " << static_cast<int>(qt_id) << std::endl;
  }
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

void parse_dri(JPEGImage &image, std::ifstream &file, int length) {
  int rst = static_cast<int>(get_2bytes(file));
  std::cerr << "  rst: " << rst << std::endl;

  image.set_rst(rst);
}

void parse_sos(JPEGImage &image, std::ifstream &file, int length) {
  // skip 1byte (1:gray scale, 3:YCrCb, 4:CMYK)
  file.seekg(1, file.cur);

  // (Y Cr Cb) each 2bytes (color_id, DC_ht_id/AC_ht_id)
  for (int i = 0; i != 3; i++) {
    unsigned char color_id = get_byte(file);
    unsigned char ht_id = get_byte(file);
    image.set_color_ht_id(color_id, ht_id);
  }

  // skip 3bytes (0x00 0x3f 0x00)
  file.seekg(3, file.cur);
}

std::string bmp_filename(std::string filename) {
  auto end = filename.find_last_of('.');
  filename = filename.substr(0, end).append(".bmp");

  return filename;
}

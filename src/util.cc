#include "util.h"

void parse(std::string filename) {
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
        length = get_length(file);
        std::cout << "APP0: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::APP1:
        length = get_length(file);
        std::cout << "APP1: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::APP2:
        length = get_length(file);
        std::cout << "APP2: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::APP3:
        length = get_length(file);
        std::cout << "APP3: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::APP4:
        length = get_length(file);
        std::cout << "APP4: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::APP5:
        length = get_length(file);
        std::cout << "APP5: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::APP6:
        length = get_length(file);
        std::cout << "APP6: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::APP7:
        length = get_length(file);
        std::cout << "APP7: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::APP8:
        length = get_length(file);
        std::cout << "APP8: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::APP9:
        length = get_length(file);
        std::cout << "APP9: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::APP10:
        length = get_length(file);
        std::cout << "APP10: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::APP11:
        length = get_length(file);
        std::cout << "APP11: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::APP12:
        length = get_length(file);
        std::cout << "APP12: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::APP13:
        length = get_length(file);
        std::cout << "APP13: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::APP14:
        length = get_length(file);
        std::cout << "APP14: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::APP15:
        length = get_length(file);
        std::cout << "APP15: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::DQT:
        length = get_length(file);
        std::cout << "DQT: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::SOF0:
        length = get_length(file);
        std::cout << "SOF0: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::DHT:
        length = get_length(file);
        std::cout << "DHT: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::DRI:
        length = get_length(file);
        std::cout << "DRI: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::SOS:
        length = get_length(file);
        std::cout << "SOS: " << length << std::endl;
        file.seekg(length, file.cur);
        break;
       case Tag::EOI:
        std::cout << "EOI" << std::endl;
        break;
      }
    }
  }
}

int get_length(std::ifstream &file) {
  unsigned char hi = 0, lo = 0;
  file.read(reinterpret_cast<char*>(&hi), 1);
  file.read(reinterpret_cast<char*>(&lo), 1);

  return static_cast<int>((hi << 8) | lo) - 2;
}

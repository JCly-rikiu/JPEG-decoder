#include <iostream>
#include <fstream>
#include <string>

namespace Tag {
  static const unsigned char START = 0xff;
  static const unsigned char SOI = 0xd8;
  static const unsigned char APP0 = 0xe0;
  static const unsigned char APP1 = 0xe1;
  static const unsigned char APP2 = 0xe2;
  static const unsigned char APP3 = 0xe3;
  static const unsigned char APP4 = 0xe4;
  static const unsigned char APP5 = 0xe5;
  static const unsigned char APP6 = 0xe6;
  static const unsigned char APP7 = 0xe7;
  static const unsigned char APP8 = 0xe8;
  static const unsigned char APP9 = 0xe9;
  static const unsigned char APP10 = 0xea;
  static const unsigned char APP11 = 0xeb;
  static const unsigned char APP12 = 0xec;
  static const unsigned char APP13 = 0xed;
  static const unsigned char APP14 = 0xee;
  static const unsigned char APP15 = 0xef;
  static const unsigned char DQT = 0xdb;
  static const unsigned char SOF0 = 0xc0;
  static const unsigned char DHT = 0xc4;
  static const unsigned char DRI = 0xdd;
  static const unsigned char SOS = 0xda;
  static const unsigned char EOI = 0xd9;
};

void parse(std::string);
int get_length(std::ifstream&);

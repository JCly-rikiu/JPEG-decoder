#ifndef UTIL
#define UTIL

#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include <vector>

#include "jpeg.h"

namespace Tag {
  static const unsigned char FF = 0xff;
  static const unsigned char SOI = 0xd8;
  static const unsigned char APP0 = 0xe0;
  static const unsigned char APP15 = 0xef;
  static const unsigned char DQT = 0xdb;
  static const unsigned char SOF0 = 0xc0;
  static const unsigned char DHT = 0xc4;
  static const unsigned char DRI = 0xdd;
  static const unsigned char SOS = 0xda;
  static const unsigned char EOI = 0xd9;
  static const unsigned char DATA = 0x00;
  static const unsigned char RST0 = 0xd0;
  static const unsigned char RST7 = 0xd7;
};

void parse(JPEGImage &, const std::string &);
unsigned char get_byte(std::ifstream &);
int get_2bytes(std::ifstream &);
void parse_dqt(JPEGImage &, std::ifstream &, int);
void parse_sof0(JPEGImage &, std::ifstream &, int);
void parse_dht(JPEGImage &, std::ifstream &, int);
void parse_sos(JPEGImage &, std::ifstream &, int);
std::string bmp_filename(std::string);

#endif

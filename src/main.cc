#include "jpeg.h"
#include "util.h"

int main(int argc, char *argv[]) {
  JPEGImage image;
  std::string str(argv[1]);
  parse(image, str);

  image.create_hts();

  return EXIT_SUCCESS;
}

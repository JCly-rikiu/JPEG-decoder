#include "jpeg.h"
#include "util.h"

int main(int argc, char *argv[]) {
  JPEGImage image;

  std::string filename(argv[1]);
  parse(image, filename);

  image.decode();

  image.save_to_bmp(bmp_filename(filename));

  return EXIT_SUCCESS;
}

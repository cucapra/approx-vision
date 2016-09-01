//
// Halide Load 3 Channel Image Benchmark
//
// This file loads a 3 channel image and then writes it
// back without doing any operations.
// Its primary purpose is to have its profile subtracted
// from each of the Halide stage profiles.

#include "Halide.h"
#include "halide_image_io.h"

int main(int argc, char** argv )
{

  using namespace Halide;
  using namespace Halide::Tools;

  // Load input image 
  Image<uint8_t> input;
  input = load_image("../benchmark_images/beer_hall.raw_3C.png");

  save_image(input, "test.png");

  return 0;

}

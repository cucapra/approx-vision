
#include "Halide.h"
#include "halide_image_io.h"
#include <opencv2/opencv.hpp>

using namespace Halide;
using namespace cv;

///////////////////////////////////////////////////////////////////////////////////////
// Conversion functions

Mat Image2Mat( Image<float> InImage );

Image<float> Mat2Image( Mat InMat );

///////////////////////////////////////////////////////////////////////////////////////
// OpenCV Funcs for camera pipeline


///////////////////////////////////////////////////////////////////////////////////////
// Halide Funcs for camera pipeline

Func make_scale( Image<uint8_t> *in_img );

Func make_descale( Image<float> *in_func);
//Func make_descale( Func *in_func );


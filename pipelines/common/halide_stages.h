
#include "Halide.h"
#include "halide_image_io.h"

using namespace Halide;

///////////////////////////////////////////////////////////////////////////////////////
// Halide Funcs for camera pipeline

Func make_scale( Image<uint8_t> *in_img );

Func make_descale( Func *in_func );


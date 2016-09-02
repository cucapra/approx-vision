///////////////////////////////////////////////////////////////
// Image Pipeline Halide Configuration File
///////////////////////////////////////////////////////////////
//
// This file defines the parameters associated with the camera
// model and the input images. These parameters can be found
// within the camera model files themselves. The format of 
// the camera model files is described at the link below. 
//
// Model format readme:
// http://www.comp.nus.edu.sg/~brown/radiometric_calibration/datasets/Model_param/readme.pdf
//
///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
// Input Image Parameters
///////////////////////////////////////////////////////////////

char image_1C[] = 
"../benchmark_images/beer_hall.raw_1C.png";

char image_3C[] =
"../benchmark_images/beer_hall.raw_3C.png";
//"../benchmark_images/input.png";

// Process the full image?
// true:  process the full image, ignore patch parameters
// false: process only the patch defined by the patch parameters
bool full = true;

///////////////////////////////////////////////////////////////
// Patch Parameters
///////////////////////////////////////////////////////////////

// The width and height of the patch (in pixels)
int patchsize = 500;

// The x and y location of the upper left corner
// of the patch to be processed
int xstart    = 551;
int ystart    = 2751; 

///////////////////////////////////////////////////////////////
// Camera Model Parameters
///////////////////////////////////////////////////////////////

// Path to the camera model to be used
char cam_model_path[] =
"../../cam_models/NikonD7000/";

// White balance index (select white balance from transform file)
// The first white balance in the file has a wb_index of 1
// For more information on model format see the readme
int wb_index = 
6;

// Number of control points
const int num_ctrl_pts = 
3702;





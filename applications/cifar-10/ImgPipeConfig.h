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

// Demosaiced raw input (to forward pipeline)
char demosaiced_image[] = 
"../../imgs/NikonD7000FL/DSC_0916.NEF.raw_1C.tiff.demosaiced.png";

// Jpeg input (to backward pipeline)
char jpg_image[] =
"../../imgs/NikonD7000FL/DSC_0916.JPG.png";

///////////////////////////////////////////////////////////////
// Camera Model Parameters
///////////////////////////////////////////////////////////////

// Path to the camera model to be used
char cam_model_path[] =
"../../../ReversiblePipeline/camera_models/NikonD7000/";

// White balance index (select white balance from transform file)
// The first white balance in the file has a wb_index of 1
// For more information on model format see the readme
int wb_index = 
6;

// Number of control points
const int num_ctrl_pts = 
3702;

///////////////////////////////////////////////////////////////
// Patches to test 
///////////////////////////////////////////////////////////////

// Patch start locations
// [xstart, ystart]
//
// NOTE: These start locations must align with the demosiac 
// pattern start if using the version of this pipeline with
// demosaicing

const int patchstarts[12][2] = {
  {551,  2751},
  {1001, 2751},
  {1501, 2751},
  {2001, 2751},
  {551,  2251},
  {1001, 2251},
  {1501, 2251},
  {2001, 2251},
  {551,  1751},
  {1001, 1751},
  {1501, 1751},
  {2001, 1751}
};

// Height and width of patches
const int patchsize = 
10;

// Number of patches to test
const int patchnum  = 
1;

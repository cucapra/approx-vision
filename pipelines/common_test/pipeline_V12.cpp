#include "PipelineUtil.h"

#include <string>
#include <iostream>

// Pipeline V12
// 
// Test type:
// Only do demosaic, denoise, and tone map
// 
// Stages:
// Rto, Rg, Rtr, Fto

int main(int argc, char **argv) {

  using namespace std;

  // Inform user of usage method
  if ( argc != 3 )
  {
      printf("usage: \n./convert path/to/in/image out/image/dir\n");
      return -1;
  }

  // Input image (path and name)
  char * in_img_path = argv[1];

  // Output image (just path)
  char * out_path    = argv[2];

  // Specify stages
  PipelineStageRev rev_stages[]   = { RevToneMap, RevGamutMap, RevTransform };
  PipelineStageCV cv_stages[]     = { };
  PipelineStageFwd fwd_stages[]   = { ToneMap };

  int num_stages[]  = { sizeof(rev_stages) / sizeof(rev_stages[0]), 
                        sizeof(cv_stages)  / sizeof(cv_stages[0]), 
                        sizeof(fwd_stages) / sizeof(fwd_stages[0]) }; 

  // run image pipeline with specified stages
  run_image_pipeline( in_img_path, 
                      out_path, 
                      rev_stages, 
                      cv_stages, 
                      fwd_stages, 
                      num_stages );
}


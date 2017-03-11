#include "PipelineUtil.h"

#include <string>
#include <iostream>

// Pipeline V49
// 
// Test type: 
// Raw data w/ linear sampling: 2 bits
// 
// Stages:
// Requant2
//
// Note: V2 must be run first. This expects input binary to be result of V2

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
  PipelineStageRev rev_stages[]   = { };
  PipelineStageCV cv_stages[]     = { };
  PipelineStageFwd fwd_stages[]   = { Requant2 };

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

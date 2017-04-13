#include "PipelineUtil.h"
#include "CameraModel.h"

#include <string>
#include <iostream>

// Pipeline V8 
// 
// Test type: 
// Skip only gamut mapping
// 
// Stages:
// Rto, Rg, Fto

int main(int argc, char **argv) {

  using namespace std;

  // Inform user of usage method
  if ( argc != 6 )
  {
      printf("usage: \n./convert path/to/in/image out/image/dir\n");
      return -1;
  }

  // Input image (path and name)
  char * in_img_path    = argv[1];

  // Output image (just path)
  char * out_path       = argv[2];

  // camera model parameters
  char * cam_model_path = argv[3];
  int wb_index          = atoi(argv[4]);
  int num_ctrl_pts      = atoi(argv[5]); 

  CameraModel *cam_model = new CameraModel(cam_model_path, wb_index, num_ctrl_pts);

  // Specify stages
  PipelineStage stages[] = { Scale, RevToneMap, RevGamutMap, ToneMap, Descale };

  int num_stages = sizeof(stages) / sizeof(stages[0]);

  // run image pipeline with specified stages
  run_image_pipeline( in_img_path, 
                      out_path, 
                      *cam_model,
                      stages,
                      num_stages );
}

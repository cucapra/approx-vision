#include "PipelineUtil.h"
#include "CameraModel.h"

#include <string>
#include <iostream>

/**
General Pipeline

Args:
    path to input image file
    path to output image directory
    path to camera model
    wb index
    num of control points
    list of stages to run deliminated by single space


  ex)
    /approx-vision/pipelines/CLI/temp/temp_in.png 
    /approx-vision/pipelines/CLI/temp/output/ 
    /approx-vision/cam_models/NikonD7000/ 
    6 
    3702 
    Scale RevToneMap RevGamutMap RevTransform ToneMap Descale
**/

using namespace std;

// mapping from string to PipelineStage enum
map<string, PipelineStage> strToPipelineStage = {
  {"Scale",  Scale},
  {"Requant1", Requant1},
  {"Requant2", Requant2},
  {"Requant3", Requant3},
  {"Requant4", Requant4},
  {"Requant5", Requant5},
  {"Requant6", Requant6},
  {"Requant7", Requant7},
  {"RevToneMap", RevToneMap},
  {"RevGamutMap", RevGamutMap},
  {"RevTransform", RevTransform},
  {"Renoise",  Renoise},
  {"Remosaic", Remosaic},
  {"GaussianBlurCV", GaussianBlurCV},
  {"LloydRequant", LloydRequant},
  {"Descale",  Descale},
  {"GamutMap", GamutMap},
  {"Transform", Transform},
  {"ToneMap",  ToneMap},
  {"DemosSubSample", DemosSubSample},
  {"DemosNN",  DemosNN},
  {"DemosInterp", DemosInterp},
  {"QrtrResBinning", QrtrResBinning} ,
  {"PwlToneMap",  PwlToneMap}
};

PipelineStage convert_stage(char * stage) {
  return strToPipelineStage.at(stage);
}


int main(int argc, char ** argv) {

  // Input image (path and name)
  char * in_img_path    = argv[1];

  // Output image (just path)
  char * out_path       = argv[2];

  // camera model parameters
  char * cam_model_path = argv[3];
  int wb_index          = atoi(argv[4]);
  int num_ctrl_pts      = atoi(argv[5]);

  CameraModel *cam_model = new CameraModel(cam_model_path, wb_index, num_ctrl_pts);

  // read in stages to run
  PipelineStage stages[argc - 6];
  for (int i = 6; i < argc; i++) {
    stages[i - 6] = convert_stage(argv[i]);
  }
  int num_stages = sizeof(stages) / sizeof(stages[0]);

  // run image pipeline with specified stages
  run_image_pipeline( in_img_path, out_path, *cam_model, stages, num_stages );
}


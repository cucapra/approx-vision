#ifndef CAMERA_MODEL_H
#define CAMERA_MODEL_H

#include "Halide.h"
#include "halide_image_io.h"
#include <opencv2/opencv.hpp>

#include "/approx-vision/pipelines/common/LoadCamModel.h"
#include "/approx-vision/pipelines/common/MatrixOps.h"


#include <vector>
#include <string>


  using namespace std;
  using namespace Halide;
  using namespace Halide::Tools;
  using namespace cv;

  // const string raw2jpg_transform = "raw2jpg_transform.txt";
  // const string raw2jpg_ctrl_pts  = "raw2jpg_ctrlPoints.txt";
  // const string jpg2raw_ctrl_pts  = "jpg2raw_ctrlPoints.txt";
  // const string raw2jpg_coefs     = "raw2jpg_coefs.txt";
  // const string jpg2raw_coefs     = "jpg2raw_coefs.txt";
  // const string jpg2raw_respFcns  = "jpg2raw_respFcns.txt";

  class CameraModel {

    char * cam_model_path; // path to camera model
    int wb_index;
    int num_ctrl_pts;

    vector<vector<float>> TsTw;
    vector<vector<float>> rev_ctrl_pts, rev_weights, rev_coefs;
    vector<vector<float>> ctrl_pts, weights, coefs;
    vector<vector<float>> rev_tone;
    vector<vector<float>> TsTw_tran;
    vector<vector<float>> TsTw_tran_inv;

    Image<float> rev_ctrl_pts_h;
    Image<float> rev_weights_h;
    Image<float> ctrl_pts_h;
    Image<float> weights_h;
    Image<float> rev_tone_h;

    void load_model_parameters();
    void convert_to_halide_image();

  public:
    CameraModel(char* camera_model_path, int white_balance_index, int num_control_points);
    
    Image<float> get_rev_ctrl_pts_h();
    Image<float> get_ctrl_pts_h();

    Image<float> get_rev_weights_h();
    Image<float> get_weights_h();

    Image<float> get_rev_tone_h();

    vector<vector<float>> get_tstw_tran_inv();
    vector<vector<float>> get_tstw_tran();

    vector<vector<float>> get_cam_rev_coefs();
    vector<vector<float>> get_cam_coefs();

    int get_num_ctrl_pts();
  };

#endif
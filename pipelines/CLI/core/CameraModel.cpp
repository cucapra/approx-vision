
#include "/approx-vision/pipelines/CLI/core/CameraModel.h"

using namespace std;

// Constructor
CameraModel::CameraModel(char* camera_model_path, int white_balance_index, int num_control_points) {

  cam_model_path  = camera_model_path;
  wb_index        = white_balance_index;
  num_ctrl_pts    = num_control_points;

  load_model_parameters();
  convert_to_halide_image();
}

void CameraModel::load_model_parameters() {
  // Load model parameters from file
  // NOTE: Ts, Tw, and TsTw read only forward data
  // ctrl_pts, weights, and coefs are either forward or backward
  // tone mapping is always backward
  // This is due to the the camera model format
  TsTw          = get_TsTw     (cam_model_path, wb_index);
  rev_ctrl_pts  = get_ctrl_pts (cam_model_path, num_ctrl_pts, 0);
  rev_weights   = get_weights  (cam_model_path, num_ctrl_pts, 0);
  rev_coefs     = get_coefs    (cam_model_path, num_ctrl_pts, 0);
  ctrl_pts      = get_ctrl_pts (cam_model_path, num_ctrl_pts, 1);
  weights       = get_weights  (cam_model_path, num_ctrl_pts, 1);
  coefs         = get_coefs    (cam_model_path, num_ctrl_pts, 1);
  rev_tone      = get_rev_tone (cam_model_path);

  // Take the transpose of the color map and white balance transform for later use
  TsTw_tran     = transpose_mat(TsTw);

  // Create an inverse of TsTw_tran
  TsTw_tran_inv = inv_3x3mat(TsTw_tran);
}

void CameraModel::convert_to_halide_image() {
   // Convert backward control points to a Halide image
  int width  = rev_ctrl_pts[0].size();
  int length = rev_ctrl_pts.size();
  rev_ctrl_pts_h = Image<float>(width,length);
  for (int y=0; y<length; y++) {
    for (int x=0; x<width; x++) {
      rev_ctrl_pts_h(x,y) = rev_ctrl_pts[y][x];
    }
  }

  // Convert backward weights to a Halide image
  width  = rev_weights[0].size();
  length = rev_weights.size();
  rev_weights_h = Image<float>(width,length);
  for (int y=0; y<length; y++) {
    for (int x=0; x<width; x++) {
      rev_weights_h(x,y) = rev_weights[y][x];
    }
  }

  // Convert control points to a Halide image
  width  = ctrl_pts[0].size();
  length = ctrl_pts.size();
  ctrl_pts_h = Image<float> (width,length);
  for (int y=0; y<length; y++) {
    for (int x=0; x<width; x++) {
      ctrl_pts_h(x,y) = ctrl_pts[y][x];
    }
  }

  // Convert weights to a Halide image
  width  = weights[0].size();
  length = weights.size();
  weights_h = Image<float>(width,length);
  for (int y=0; y<length; y++) {
    for (int x=0; x<width; x++) {
      weights_h(x,y) = weights[y][x];
    }
  }

  // Convert the reverse tone mapping function to a Halide image
  width  = 3;
  length = 256;
  rev_tone_h = Image<float>(width,length);
  for (int y=0; y<length; y++) {
    for (int x=0; x<width; x++) {
      rev_tone_h(x,y) = rev_tone[y][x];
    }
  } 
}

Image<float> CameraModel::get_rev_ctrl_pts_h() {
  return rev_ctrl_pts_h;
}

Image<float> CameraModel::get_ctrl_pts_h() {
  return ctrl_pts_h;
}

Image<float> CameraModel::get_rev_weights_h() {
  return rev_weights_h;
}

Image<float> CameraModel::get_weights_h() {
  return weights_h;
}

Image<float> CameraModel::get_rev_tone_h() {
  return rev_tone_h;
}

vector<vector<float>> CameraModel::get_tstw_tran_inv() {
  return TsTw_tran_inv;
}

vector<vector<float>> CameraModel::get_tstw_tran() {
  return TsTw_tran;
}

vector<vector<float>> CameraModel::get_cam_rev_coefs() {
  return rev_coefs;
}

vector<vector<float>> CameraModel::get_cam_coefs() {
  return coefs;
}

int CameraModel::get_num_ctrl_pts() {
  return num_ctrl_pts;
}
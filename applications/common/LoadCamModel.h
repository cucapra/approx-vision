#include <vector>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

// Get color space transform
vector<vector<float>> get_Ts(char* cam_model_path);

// Get white balance transform
vector<vector<float>> get_Tw(char* cam_model_path, int wb_index);

// Get combined transforms for checking
vector<vector<float>> get_TsTw(char* cam_model_path, int wb_index);

// Get control points
vector<vector<float>> get_ctrl_pts(char* cam_model_path, int num_cntrl_pts, bool direction);

// Get weights
vector<vector<float>> get_weights(char* cam_model_path, int num_cntrl_pts, bool direction);

// Get coeficients 
vector<vector<float>> get_coefs(char* cam_model_path, int num_cntrl_pts, bool direction);

// Get reverse tone mapping
vector<vector<float>> get_rev_tone(char* cam_model_path);

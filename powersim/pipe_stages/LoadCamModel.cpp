#include "LoadCamModel.h"

using namespace std;

// Get color space transform
vector<vector<float>> get_Ts(char* cam_model_path) {

  vector<vector<float>> Ts;
  string   line;
  int line_idx = 0;

  // Open file for reading
  ifstream file(cam_model_path+string("raw2jpg_transform.txt"));

  // Read a line at a time
  while(getline(file, line))
  {
    vector<float> lineData, temp;
    stringstream  lineStream(line);
    float value;
    // Read one value at a time from the line
    while(lineStream >> value)
    {
      lineData.push_back(value);
    }
    if (line_idx>=1 && line_idx<=3) {
      for (int i=0; i<3; i++) {
        temp.push_back(lineData[i]);
      }
      Ts.push_back(temp);      
    }
    line_idx = line_idx + 1;
  }

  return Ts;

}

// Get white balance transform
vector<vector<float>> get_Tw(char* cam_model_path, int wb_index) {

  vector<vector<float>> Tw;
  string   line;
  int line_idx = 0;

  // Calculate base for the white balance transform selected
  // For more details see the camera model readme
  int wb_base  = 8 + 5*(wb_index-1);

  // Open file for reading
  ifstream file(cam_model_path+string("raw2jpg_transform.txt"));

  // Read a line at a time
  while(getline(file, line))
  {
    vector<float> lineData, temp;
    stringstream  lineStream(line);
    float value;
    // Read one value at a time from the line
    while(lineStream >> value)
    {
      lineData.push_back(value);
    }
    // If this is the line with the white balance
    if (line_idx==wb_base) {
      // Convert the white balance vector into a diagaonal matrix
      for (int i=0; i<3; i++) {
        for (int j=0; j<3; j++) {
          if (i==j) { temp.push_back(lineData[i]); }
          else      { temp.push_back(0.0);         }
        }
        Tw.push_back(temp);
        temp.clear();
      }
          
    }
    line_idx = line_idx + 1;
  }

  return Tw;

}

// Get combined transforms for checking
vector<vector<float>> get_TsTw(char* cam_model_path, int wb_index) {

  vector<vector<float>> TsTw;
  string   line;
  int line_idx = 0;

  // Calculate base for the white balance transform selected
  // For more details see the camera model readme
  int wb_base  = 5 + 5*(wb_index-1);

  // Open file for reading
  ifstream file(cam_model_path+string("raw2jpg_transform.txt"));

  // Read a line at a time
  while(getline(file, line))
  {
    vector<float> lineData, temp;
    stringstream  lineStream(line);
    float value;
    // Read one value at a time from the line
    while(lineStream >> value)
    {
      lineData.push_back(value);
    }
    if (line_idx>=wb_base && line_idx<=(wb_base+2)) {
      for (int i=0; i<3; i++) {
        temp.push_back(lineData[i]);
      }
      TsTw.push_back(temp);      
    }
    line_idx = line_idx + 1;
  }

  return TsTw;

}

// Get control points
vector<vector<float>> get_ctrl_pts(char* cam_model_path, int num_cntrl_pts, bool direction) {

  vector<vector<float>> ctrl_pnts;
  string   line, directionfile;
  int line_idx = 0;

  // Open file for reading
  if (direction==1) {
    // Forward pipeline
    directionfile = string("raw2jpg_ctrlPoints.txt");
  } else {
    // Backward pipeline
    directionfile = string("jpg2raw_ctrlPoints.txt");
  }
  ifstream file(cam_model_path+directionfile);

  // Read a line at a time
  while(getline(file, line))
  {
    vector<float> lineData, temp;
    stringstream  lineStream(line);
    float value;
    // Read one value at a time from the line
    while(lineStream >> value)
    {
      lineData.push_back(value);
    }
    if (line_idx>=1 && line_idx<=num_cntrl_pts) {
      for (int i=0; i<3; i++) {
        temp.push_back(lineData[i]);
      }
      ctrl_pnts.push_back(temp);      
    }
    line_idx = line_idx + 1;
  }

  return ctrl_pnts;

}

// Get weights
vector<vector<float>> get_weights(char* cam_model_path, int num_cntrl_pts, bool direction) {

  vector<vector<float>> weights;
  string   line, directionfile;
  int line_idx = 0;

  // Open file for reading
  if (direction==1) {
    // Forward pipeline
    directionfile = string("raw2jpg_coefs.txt");
  } else {
    // Backward pipeline
    directionfile = string("jpg2raw_coefs.txt");
  }
  ifstream file(cam_model_path+directionfile);

  // Read a line at a time
  while(getline(file, line))
  {
    vector<float> lineData, temp;
    stringstream  lineStream(line);
    float value;
    // Read one value at a time from the line
    while(lineStream >> value)
    {
      lineData.push_back(value);
    }
    if (line_idx>=1 && line_idx<=num_cntrl_pts) {
      for (int i=0; i<3; i++) {
        temp.push_back(lineData[i]);
      }
      weights.push_back(temp);      
    }
    line_idx = line_idx + 1;
  }

  return weights;

}

// Get coeficients 
vector<vector<float>> get_coefs(char* cam_model_path, int num_cntrl_pts, bool direction) {

  vector<vector<float>> coefs;
  string   line, directionfile;
  int line_idx = 0;

  // Open file for reading
  if (direction==1) {
    // Forward pipeline
    directionfile = string("raw2jpg_coefs.txt");
  } else {
    // Backward pipeline
    directionfile = string("jpg2raw_coefs.txt");
  }
  ifstream file(cam_model_path+directionfile);

  // Read a line at a time
  while(getline(file, line))
  {
    vector<float> lineData, temp;
    stringstream  lineStream(line);
    float value;
    // Read one value at a time from the line
    while(lineStream >> value)
    {
      lineData.push_back(value);
    }
    if (line_idx>=(num_cntrl_pts+1) && line_idx<=(num_cntrl_pts+4)) {
      for (int i=0; i<3; i++) {
        temp.push_back(lineData[i]);
      }
      coefs.push_back(temp);      
    }
    line_idx = line_idx + 1;
  }

  return coefs;

}

// Get reverse tone mapping
vector<vector<float>> get_rev_tone(char* cam_model_path) {

  vector<vector<float>> rev_tone;
  string   line;
  int line_idx = 0;

  // Open file for reading
  ifstream file(cam_model_path+string("jpg2raw_respFcns.txt"));

  // Read a line at a time
  while(getline(file, line))
  {
    vector<float> lineData, temp;
    stringstream  lineStream(line);
    float value;
    // Read one value at a time from the line
    while(lineStream >> value)
    {
      lineData.push_back(value);
    }
    if (line_idx>=1 && line_idx<=256) {
      for (int i=0; i<3; i++) {
        temp.push_back(lineData[i]);
      }
      rev_tone.push_back(temp);      
    }
    line_idx = line_idx + 1;
  }

  return rev_tone;

}



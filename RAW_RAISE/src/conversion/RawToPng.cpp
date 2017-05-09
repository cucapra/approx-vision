/////////////////////////////////////////////////////////////////////////
//
//  This script is built to preprocess raw files for both the Halide and 
//  Matlab reversible imaging pipelines. Raw files are used as input
//  to the forward imaging pipelines, and as reference output for the 
//  the backward imaging pipelines. Since neither Halide nor Matlab can
//  read raw file formats (such as Nikon .NEF), LibRaw is used to 
//  receive raw data, and OpenCV is used to scale, preprocess as needed,
//  and write readable files.
//
//  Input:
//    Filepath to raw image input file (.NEF)
//    Filepath to png image output file
//    Number of bits used to represent raw image input file
//    
//  Output:
//    Saves scaled 32 bit float point 3 channel image to output file path
//
//
/////////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <opencv2/opencv.hpp>
#include <libraw/libraw.h>

#include "/approx-vision/pipelines/CLI/core/PipelineUtil.h"

using namespace std;
using namespace cv;


int run_image_convert_pipeline( Image<float> *inImage, 
                            char* out_img_path,
                            enum PipelineStage stages[],
                            int num_stages ) {

  /////////////////////////////////////////////////////////////////////////////
  //                        Import and format model data
  /////////////////////////////////////////////////////////////////////////////
  char cam_model_path[] = "/approx-vision/cam_models/NikonD7000/";
  int wb_index          = 6;
  int ctrl_pts          = 3702;
  CameraModel * cam_model = new CameraModel(cam_model_path, wb_index, ctrl_pts);


  int num_ctrl_pts                    = (cam_model)->get_num_ctrl_pts();

  vector<vector<float>> rev_coefs     = (cam_model)->get_cam_rev_coefs();
  vector<vector<float>> coefs         = (cam_model)->get_cam_coefs();
  vector<vector<float>> TsTw_tran     = (cam_model)->get_tstw_tran();
  vector<vector<float>> TsTw_tran_inv = (cam_model)->get_tstw_tran_inv();

  Image<float> rev_ctrl_pts_h         = (cam_model)->get_rev_ctrl_pts_h();
  Image<float> rev_weights_h          = (cam_model)->get_rev_weights_h();
  Image<float> ctrl_pts_h             = (cam_model)->get_ctrl_pts_h();
  Image<float> weights_h              = (cam_model)->get_weights_h();
  Image<float> rev_tone_h             = (cam_model)->get_rev_tone_h();

  /////////////////////////////////////////////////////////////////////////////
  //                    Import and format input image
  /////////////////////////////////////////////////////////////////////////////

  int width                           = (*inImage).width();
  int height                          = (*inImage).height();

  /////////////////////////////////////////////////////////////////////////////
  //                          Camera Pipeline
  /////////////////////////////////////////////////////////////////////////////

  Func resultFunc                       = make_Image2Func( inImage );

  vector<int> qrtr_bin_factor = { 1 };

  // run stages
  resultFunc = run_image_pipeline_stage(&resultFunc,
                                        stages,
                                        num_stages,
                                        width,
                                        height,
                                        num_ctrl_pts,
                                        &rev_ctrl_pts_h,
                                        &rev_tone_h,
                                        &rev_weights_h,
                                        &rev_coefs,
                                        &TsTw_tran_inv,
                                        &ctrl_pts_h,
                                        qrtr_bin_factor,
                                        &rev_tone_h,
                                        &weights_h,
                                        &coefs,
                                        &TsTw_tran);

  debug_print("qrtr_bin_factor: " + to_string(qrtr_bin_factor[0]));

  /////////////////////////////////////////////////////////////////////////////
  //                              Scheduling
  /////////////////////////////////////////////////////////////////////////////

  // Use JIT compiler
  resultFunc.compile_jit();
  Image<uint8_t> output         = resultFunc.realize( width / qrtr_bin_factor[0],
                                                      height/ qrtr_bin_factor[0],
                                                      3 );

  /////////////////////////////////////////////////////////////////////////////
  //                            Save the output
  /////////////////////////////////////////////////////////////////////////////
  save_image( output, (std::string(out_img_path) ).c_str() );

  return 0;
}

int main(int argc, char** argv )
{
  
  if ( argc != 6 )
  {
      printf("usage: \n./RawToPng <path/to/input/image> <path/to/output/raw/image> <path/to/output/full/image> <resize factor> <raw file bitdepth>\n");
      return -1;
  }

  // LibRaw raw data extraction 
  // Establish use of LibRaw
  LibRaw RawProcessor;
  #define imgdata RawProcessor.imgdata

  // Function failure flag
  int ret;

  // Path to input and output
  char * in_path = argv[1];
  char * out_path_raw = argv[2];
  char * out_path_full = argv[3];

  int resize_factor = atoi(argv[4]);

  // Bitdepth of raw input
  int num_raw_bits = atoi(argv[5]);

  // Read in raw image with LibRaw
  if ((ret = RawProcessor.open_file(in_path)) != LIBRAW_SUCCESS)
  {
      fprintf(stderr, in_path, libraw_strerror(ret));
      return -1; 
  }

  // Unpack the raw image, storing it in imgdata.rawdata.raw_image
  if ((ret = RawProcessor.unpack()) != LIBRAW_SUCCESS)
  {
      return -1; 
  }
 
  // Extract raw data stored in 16 bit, 1 channel image
  Mat raw_1C = Mat(
    imgdata.sizes.raw_height,
    imgdata.sizes.raw_width,
    CV_16UC1,
    imgdata.rawdata.raw_image
  );

  // Scale the data to fit the 16 bit representation
  int scale = 1 << (16-num_raw_bits);
  raw_1C = raw_1C * scale;

  // resize for faster conversion
  int height_resize = imgdata.sizes.raw_height / resize_factor * 2;
  int width_resize = imgdata.sizes.raw_width / resize_factor * 2;
  // cout << imgdata.sizes.raw_height << endl;
  // cout << imgdata.sizes.raw_width << endl;
  // cout << height_resize << endl;
  // cout << width_resize << endl;

  Mat raw_3C = Mat(
    height_resize, 
    width_resize, 
    CV_32FC3  
  );
  vector<Mat> three_channels;
  split(raw_3C, three_channels);

  // 1 channel to 3 channel
  // Note: OpenCV stores as BGR not RGB
  float scale_float = pow(2, 16);
  int color_channel = 0;
  int row = 0;
  int col = 0;

  for (int y = 0; y < raw_1C.rows; y += (resize_factor*2)) {
    for (int x = 0; x < raw_1C.cols; x += (resize_factor*2)) {
      row = y / (resize_factor) * 2;
      col = x / (resize_factor) * 2;

      three_channels[1].at<float>(row, col) 
            = (float)(raw_1C.at<unsigned short>(y, x)) / scale_float;

      three_channels[2].at<float>(row, col+1) 
            = (float)(raw_1C.at<unsigned short>(y, x+1)) / scale_float;

      three_channels[0].at<float>(row+1, col) 
            = (float)(raw_1C.at<unsigned short>(y+1, x)) / scale_float;

      three_channels[1].at<float>(row+1, col+1) 
            = (float)(raw_1C.at<unsigned short>(y+1, x+1)) / scale_float;
    }
  }


  merge(three_channels, raw_3C);

  // run RAW
  // mat to image
  Image<float> inImg = Mat2Image( &raw_3C );
  Image<float> inImg2 = Mat2Image( &raw_3C );
  PipelineStage raw_stages[1] = {Descale};
  run_image_convert_pipeline(&inImg, out_path_raw, raw_stages, 1);

  // run FULL
  // mat to image
  PipelineStage full_stages[5] = {DemosSubSample, Transform, GamutMap, ToneMap, Descale};
  run_image_convert_pipeline(&inImg2, out_path_full, full_stages, 5);

  return 0;
}


/////////////////////////////////////////////////////////////////////////
//
// This script uses OpenCV to apply CLAHE to CIFAR-10 images
//
/////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <vector>
#include <fstream>
#include <stdio.h>
#include <math.h>
#include <opencv2/opencv.hpp>

int main(int argc, char **argv) {

  using namespace std;  
  using namespace cv;

  // Inform user of usage method
  if ( argc != 3 ) 
  {
      printf("usage: \n./convert in/data/name.bin out/data/name.bin \n");
      return -1; 
  }

  // Input data directory
  const char * in_data_path   = argv[1];
  // Output data directory
  const char * out_data_path  = argv[2];


  // Establish IO
  char val, label;
  float img_sum, img_avg, total_sum, total_avg;
  fstream infile(in_data_path);

  fstream outfile;
  outfile.open(out_data_path,fstream::out);


  // Loop through images
  for (int i=0; i<10000; i++) { //i<10000

    //////////////////////////////////////////////////////

    Mat original  = Mat::zeros(32,32,CV_8UC3);

    // Read in label
    infile.read(&val,1);
    label = val;
    
    img_sum = 0;

    for (int c=0; c<3; c++) { 
      for (int y=0; y<32; y++) {
        for (int x=0; x<32; x++) {
          infile.read(&val,1);
          // Swap red and blue channels since OpenCV
          // stores as bgr not rgb
          int c_bgr = c;
          if      (c==0) { c_bgr = 2; }
          else if (c==2) { c_bgr = 0; }
          original.at<Vec3b>(y,x)[c_bgr] = (unsigned char)val;
        }
      }
    }

//    imwrite( "original.png", original );

    //////////////////////////////////////////////////////

    Mat lab_img;

    cvtColor(original,lab_img,CV_BGR2YCrCb);

    vector<Mat> channels;
    split(lab_img,channels);

    // Equalize the histogram of the y channel 
    equalizeHist(channels[0], channels[0]);


/*
    // apply the CLAHE algorithm to the L channel
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
    clahe->setClipLimit(2);
    clahe->setTilesGridSize(Size(4,4));
    cv::Mat dst;
    clahe->apply(channels[0], dst);
    dst.copyTo(channels[0]);

*/

    // Merge the channels    
    Mat result;
    merge(channels,lab_img);


//    cvtColor(lab_img,result,CV_YCrCb2BGR);

    //////////////////////////////////////////////////////

//    imwrite( "CLAHE.png", result );

    // Save the output
    // Write the label
    val = label;
    outfile.write(&val,1);
    for (int c=0; c<3; c++) {
      for (int y=0; y<32; y++) {
        for (int x=0; x<32; x++) {
          // Swap red and blue channels since OpenCV
          // stores as bgr not rgb
          int c_bgr = c;
          if      (c==0) { c_bgr = 2; }
          else if (c==2) { c_bgr = 0; } 
          val = lab_img.at<Vec3b>(y,x)[c_bgr];
          outfile.write(&val,1);
        }
      }
    }     
  }

  return 0;
}


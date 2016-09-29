//
// OpenCV Re-noising
//
// This file adds noise based on parameters for
// the Nikon D80 sensor

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv; 

double getPSNR(const Mat& I1, const Mat& I2)
{
    Mat s1;
    absdiff(I1, I2, s1);       // |I1 - I2|
    s1.convertTo(s1, CV_32F);  // cannot make a square on 8 bits
    s1 = s1.mul(s1);           // |I1 - I2|^2

    Scalar s = sum(s1);         // sum elements per channel

    double sse = s.val[0] + s.val[1] + s.val[2]; // sum channels

    if( sse <= 1e-10) // for small values return zero
        return 0;
    else
    {
        double  mse =sse /(double)(I1.channels() * I1.total());
        double psnr = 10.0*log10((255*255)/mse);
        return psnr;
    }
}

int main(int argc, char** argv )
{
  // Receive input
  Mat in_img;
  in_img = imread("../powersim/benchmark_images/beer_hall.jpg");

  // Establish matrices input and output
  Mat in_double(in_img.rows,in_img.cols,CV_64FC3);
  // Scale from 8 to 12 bit representation
  // This is because noise parameters are for 12 bit
  in_img.convertTo(in_double,CV_64FC3,4096.0/255.0);
  vector<Mat> three_channels;
  split(in_double, three_channels);

  Mat out_double(in_img.rows,in_img.cols,CV_64FC3);
  Mat out_8bit(in_img.rows,in_img.cols,CV_8UC3);
  
  // Noise parameters
  // noised_pixel = unnoised_pixel + 
  //                  gaussian_rand( std_dev = sqrt(a*unnoised_pixel + b) )
  // a and b values vary between channels
  // "Image Sensor Noise Parameter Estimation by Variance Stabilization and
  // Normality Assesment"
  double red_a, red_b, green_a, green_b, blue_a, blue_b;

/*  red_a   =  0.1460;
  red_b   =  7.6876;
  green_a =  0.1352;
  green_b =  5.0834;
  blue_a  =  0.1709;
  blue_b  = 12.3381;*/

  red_a   =  20.1460;
  red_b   =  25.6876;
  green_a =  red_a;
  green_b =  red_b;
  blue_a  =  red_a;
  blue_b  =  red_b;


  double noise_;
  
  noise_ = (blue_a * 2500.0 + blue_b   );
  //printf("%f/n/n",noise_);

  // Define the random number generator
  RNG rng(0xDEADBEEF);
 
  double red_std, green_std, blue_std;
  for (int y=0; y<in_img.rows; y++) {
    for (int x=0; x<in_img.cols; x++) {
      // Compute the channel noise standard deviation
      red_std   = sqrt( 
        (red_a   * three_channels[2].at<double>(y,x)) + red_b   );
      green_std = sqrt( 
        (green_a * three_channels[1].at<double>(y,x)) + green_b );
      blue_std  = sqrt( 
        (blue_a  * three_channels[0].at<double>(y,x)) + blue_b  );

      // NOTE: OpenCV stores in BGR order (not RGB)
      // Blue channel
      three_channels[0].at<double>(y,x) = three_channels[0].at<double>(y,x) + 
        (rng.gaussian(blue_std));
      // Green channel
      three_channels[1].at<double>(y,x) = three_channels[1].at<double>(y,x) + 
        (rng.gaussian(green_std));
      // Red channel
      three_channels[2].at<double>(y,x) = three_channels[2].at<double>(y,x) + 
        (rng.gaussian(red_std));

      //noise_ = rescale * rng.gaussian(blue_std);
      //printf("%f\n",noise_);

    }
  }

  merge(three_channels, out_double);
  out_double.convertTo(out_8bit,CV_8UC3,255.0/4096.0);


/*
  // Debugging code
  double min_double, max_double;

  minMaxIdx(in_img,&min_double,&max_double);
  cout << "in_img max val: " << max_double << endl;

  minMaxIdx(in_double,&min_double,&max_double);
  Mat in_img;

  minMaxIdx(out_double,&min_double,&max_double);
  cout << "out max val: " << max_double << endl;
*/


  double psnr;

  // Get psnr of noised vs un-noised
  psnr = getPSNR(in_img, out_8bit);
  printf("%f\n",psnr);


  // Denoising experiment
  Mat denoise_in(in_img.rows,in_img.cols,CV_8UC3);
  Mat denoise_out(in_img.rows,in_img.cols,CV_8UC3);

  denoise_in = out_8bit;

  fastNlMeansDenoisingColored(denoise_in,denoise_out);
  
  psnr = getPSNR(in_img, denoise_out);
  printf("%f\n",psnr);

  imwrite("out.png",out_8bit);

  return 0;
}

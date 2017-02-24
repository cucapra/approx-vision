
#include "pipe_stages.h"

using namespace Halide;
using namespace cv;
using namespace std;

///////////////////////////////////////////////////////////////////////////////////////
// Conversion functions

Mat Image2Mat( Image<float> *InImage ) {
  int height = (*InImage).height();
  int width  = (*InImage).width();
  Mat OutMat(height,width,CV_32FC3);
  vector<Mat> three_channels;
  split(OutMat, three_channels);

  // Convert from planar RGB memory storage to interleaved BGR memory storage
  for (int y=0; y<height; y++) {
    for (int x=0; x<width; x++) {
      // Blue channel
      three_channels[0].at<float>(y,x) = (*InImage)(x,y,2);
      // Green channel
      three_channels[1].at<float>(y,x) = (*InImage)(x,y,1);
      // Red channel
      three_channels[2].at<float>(y,x) = (*InImage)(x,y,0);
    }
  }

  merge(three_channels, OutMat);

  return OutMat;
}

Image<float> Mat2Image( Mat *InMat ) {
  int height = (*InMat).rows;
  int width  = (*InMat).cols;
  Image<float> OutImage(width,height,3);
  vector<Mat> three_channels;
  split((*InMat), three_channels);

  // Convert from interleaved BGR memory storage to planar RGB memory storage
  for (int y=0; y<height; y++) {
    for (int x=0; x<width; x++) {
      // Blue channel
      OutImage(x,y,2) = three_channels[0].at<float>(y,x);
      // Green channel
      OutImage(x,y,1) = three_channels[1].at<float>(y,x);
      // Red channel
      OutImage(x,y,0) = three_channels[2].at<float>(y,x);
    }
  }

  return OutImage;
}

Func make_Image2Func ( Image<float> *InImage ) {
  Var x, y, c;
  Func Image2Func("Image2Func");
    Image2Func(x,y,c) = (*InImage)(x,y,c);
  return Image2Func;
}

///////////////////////////////////////////////////////////////////////////////////////
// OpenCV Funcs for camera pipeline

float enforce_range (float in) {

  float out = in;
  if (in < 0) {
    out = 0;
  }
  if (in > 1) {
    out = 1;
  }

  return out;
}


void OpenCV_renoise ( Mat *InMat ) {
  // "Image informative maps for componentwise estimating parameters of 
  // signal-dependent noise"

  int height = (*InMat).rows;
  int width  = (*InMat).cols;

  // Establish matrices input and output
  Mat in_double(height,width,CV_64FC3);
  // This is because noise parameters are for 12 bit
  (*InMat).convertTo(in_double,CV_64FC3,4096.0/1.0);
  vector<Mat> three_channels;
  split(in_double, three_channels);

  Mat out_double(height,width,CV_64FC3);
  Mat out_float(height,width,CV_32FC3);

  // noised_pixel = unnoised_pixel + 
  //                  gaussian_rand( std_dev = sqrt(a*unnoised_pixel + b) )
  // a and b values vary between channels
  double red_a, red_b, green_a, green_b, blue_a, blue_b;

  red_a   =  0.1460;
  red_b   =  7.6876;
  green_a =  0.1352;
  green_b =  5.0834;
  blue_a  =  0.1709;
  blue_b  = 12.3381;

/*
  red_a   =  1;
  red_b   =  10;
  green_a =  red_a;
  green_b =  red_b;
  blue_a  =  red_a;
  blue_b  =  red_b;
*/

  // Define the random number generator
  RNG rng(0xDEADBEEF);

  double red_std, green_std, blue_std;
  for (int y=0; y<height; y++) {
    for (int x=0; x<width; x++) {
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

    }
  }

  merge(three_channels, out_double);
  out_double.convertTo(out_float,CV_32FC3,1.0/4096.0);
  
  *InMat = out_float;
}


void OpenCV_gaussian_blur ( Mat *InMat ) {

  Mat OutMat;
  cv::GaussianBlur(*InMat,OutMat, Size(3,3), 0.5, 0);
  *InMat = OutMat;

}

void OpenCV_remosaic (Mat *InMat ) {

  vector<Mat> three_channels;
  cv::split((*InMat), three_channels);

  // Re-mosaic aka re-bayer the image
  // B G
  // G R

  // Note: OpenCV stores as BGR not RGB
  for (int y=0; y<(*InMat).rows; y++) {
    for (int x=0; x<(*InMat).cols; x++) {
      // If an even row
      if ( y%2 == 0 ) {
        // If an even column
        if ( x%2 == 0 ) {
          // Green pixel, remove blue and red
          three_channels[0].at<float>(y,x) = 0; 
          three_channels[2].at<float>(y,x) = 0;
          // Also divide the green by half to account
          // for interpolation reversal
          three_channels[1].at<float>(y,x) = 
            three_channels[1].at<float>(y,x) / 2;
        }
        // If an odd column
        else {
          // Red pixel, remove blue and green
          three_channels[0].at<float>(y,x) = 0;
          three_channels[1].at<float>(y,x) = 0;
        }
      }
      // If an odd row
      else {
        // If an even column
        if ( x%2 == 0 ) {
          // Blue pixel, remove red and green
          three_channels[2].at<float>(y,x) = 0;
          three_channels[1].at<float>(y,x) = 0;
        }
        // If an odd column
        else {
          // Green pixel, remove blue and red
          three_channels[0].at<float>(y,x) = 0;
          three_channels[2].at<float>(y,x) = 0;
          // Also divide the green by half to account
          // for interpolation reversal
          three_channels[1].at<float>(y,x) = 
            three_channels[1].at<float>(y,x) / 2;
        }
      }
    }
  }
  cv::merge(three_channels, *InMat);

}

void OpenCV_lloydmax_requant (Mat *InMat ) {
  vector<Mat> three_channels;
  cv::split((*InMat), three_channels);

  static const int num_levels = 256;
  float levels[num_levels];

  // Read in the levels
  ifstream file;
	file.open("../../analysis/lloydmax_b_CDF.txt");
	if(file.is_open())  {
     for(int i = 0; i < num_levels; ++i)  {
			 file >> levels[i];
	   }
	}
	// Scale the levels down to 0-1 range
	for(int i=0; i<num_levels; i++) {
    levels[i] = levels[i] / 256.0;
	}
  
	// Requantize
  for (int y=0; y<(*InMat).rows; y++) {
    for (int x=0; x<(*InMat).cols; x++) {
      for (int c=0; c<3; c++) {
        auto pos = std::upper_bound(levels, 
						                        levels + 256,
					                          three_channels[c].at<float>(y,x))
				                                                   	- levels;
        three_channels[c].at<float>(y,x) = (float)(pos) / 256.0;
			}
		}
	}
	cv::merge(three_channels, *InMat); 

}

///////////////////////////////////////////////////////////////////////////////////////
// Halide Funcs for camera pipeline

Func make_scale( Image<uint8_t> *in_img ) {
  Var x, y, c;
  // Cast input to float and scale from 8 bit 0-255 range to 0-1 range
  Func scale("scale");
    scale(x,y,c) = cast<float>( (*in_img)(x,y,c) ) / 255;
  return scale;
}

Func make_descale( Func *in_func ) {
  Var x, y, c;
  // de-scale from 0-1 range to 0-255 range, and cast to 8 bit 
  Func descale("descale");
    descale(x,y,c) = cast<uint8_t>( min( max( (*in_func)(x,y,c) * 255, 0), 255 ) );
  return descale;
}

Func make_requant( Image<uint8_t> *in_img, int num_bits ) {
  Var x, y, c;
  int scale_val = pow(2,(8-num_bits));

  Func requant("requant");
    Expr right_shift = (*in_img)(x,y,c) / scale_val;
    requant(x,y,c)   = right_shift * scale_val;

  return requant;
}

Func make_requant( Func *in_func, int num_bits ) {
  Var x, y, c;
  int scale_val = pow(2,(8-num_bits));

  Func requant("requant");
    Expr right_shift = (*in_func)(x,y,c) / scale_val;
    requant(x,y,c)   = cast<uint8_t>(right_shift * scale_val);

  return requant;  
}


Func make_demosaic_subsample( Func *in_func ) {

  Var x, y, c;
  Func demosaic_subsample("demosaic_subsample");

  // G R
  // B G

  demosaic_subsample(x,y,c) = 
    select(
      // Top
      y%2==0 && x%2==0,
        select(
          c==0, (*in_func)(x+1,y,c),  //Red
          c==1, (*in_func)(x,y,c)*2,  //Green
                (*in_func)(x,y+1,c)), //Blue
      // Red
      y%2==0 && x%2==1,
        select(
          c==0, (*in_func)(x,y,c),                       //Red
          c==1, (*in_func)(x,y+1,c)+(*in_func)(x-1,y,c), //Green
                (*in_func)(x-1,y+1,c)),                  //Blue
      // Blue
      y%2==1 && x%2==0,
        select(
          c==0, (*in_func)(x+1,y-1,c),                   //Red
          c==1, (*in_func)(x,y+1,c)+(*in_func)(x-1,y,c), //Green
                (*in_func)(x,y,c)),                      //Blue
      // Bottom green
        select(
          c==0, (*in_func)(x,y-1,c), //Red
          c==1, (*in_func)(x,y,c)*2, //Green
                (*in_func)(x-1,y,c)) //Blue
    );

  return demosaic_subsample;
}

Func make_demosaic_nn( Func *in_func ) {

  // Nearest neighbor demosaicing
  Var x, y, c;
  Func demosaic_nn("demosaic_nn");

  // G R
  // B G

  demosaic_nn(x,y,c) = 
    select(
      // Top green
      y%2==0 && x%2==0,
        select(
          c==0, (*in_func)(x+1,y,c),  //Red
          c==1, (*in_func)(x,y,c)*2,  //Green
                (*in_func)(x,y+1,c)), //Blue
      // Red
      y%2==0 && x%2==1,
        select(
          c==0, (*in_func)(x,y,c),     //Red
          c==1, (*in_func)(x-1,y,c)*2, //Green
                (*in_func)(x-1,y+1,c)),//Blue
      // Blue
      y%2==1 && x%2==0,
        select(
          c==0, (*in_func)(x+1,y-1,c), //Red
          c==1, (*in_func)(x-1,y,c)*2, //Green
                (*in_func)(x,y,c)),    //Blue
      // Top green
        select(
          c==0, (*in_func)(x,y-1,c), //Red
          c==1, (*in_func)(x,y,c)*2, //Green
                (*in_func)(x-1,y,c)) //Blue
    );

  return demosaic_nn;
}

Func make_demosaic_interp( Func *in_func ) {

  // Interpolation demosaicing
  Var x, y, c;
  Func demosaic_interp("demosaic_interp");

  // G R
  // B G

  demosaic_interp(x,y,c) = 
    select(
      // Top green
      y%2==0 && x%2==0,
         select(
          c==0, ((*in_func)(x-1,y,c) + 
                 (*in_func)(x+1,y,c))/2 , //Red
          c==1, (*in_func)(x,y,c)*2, //Green
                ((*in_func)(x,y-1,c) + 
                 (*in_func)(x,y+1,c))/2 ),//Blue
      // Red
      y%2==0 && x%2==1,
        select(
          c==0, (*in_func)(x,y,c),     //Red
          c==1, ((*in_func)(x+1,y,c) + 
                 (*in_func)(x-1,y,c) +
                 (*in_func)(x,y+1,c) +
                 (*in_func)(x,y-1,c))/2 , //Green
                ((*in_func)(x+1,y-1,c) + 
                 (*in_func)(x-1,y+1,c) +
                 (*in_func)(x+1,y+1,c) +
                 (*in_func)(x-1,y-1,c))/4 ),//Blue
      // Blue
      y%2==1 && x%2==0,
        select(
          c==0, ((*in_func)(x+1,y-1,c) + 
                 (*in_func)(x-1,y+1,c) +
                 (*in_func)(x+1,y+1,c) +
                 (*in_func)(x-1,y-1,c))/4 , //Red
          c==1, ((*in_func)(x+1,y,c) + 
                 (*in_func)(x,y+1,c) +
                 (*in_func)(x-1,y,c) +
                 (*in_func)(x,y+1,c))/2 , //Green
                (*in_func)(x,y,c) ),    //Blue
      // Bottom Green
        select(
          c==0, ((*in_func)(x,y-1,c) + 
                 (*in_func)(x,y+1,c))/2 , //Red
          c==1, (*in_func)(x,y,c)*2, //Green
                ((*in_func)(x-1,y,c) + 
                 (*in_func)(x+1,y,c))/2 )//Blue

    );

  return demosaic_interp;
}

Func make_qrtr_res_binning( Func *in_func ) {

  Var x, y, c;

  Func qrtr_res_binning("qrtr_res_binning");

  qrtr_res_binning(x,y,c) = ( (*in_func)( x*2   ,(y*2)+1,c) +
                              (*in_func)((x*2)+1, y*2   ,c) +
                              (*in_func)( x*2   , y*2   ,c) +
                              (*in_func)((x*2)+1,(y*2)+1,c) ) / 4;

  return qrtr_res_binning;
}

Func make_rev_tone_map( Func *in_func, 
                       Image<float> *rev_tone_h ) {
  Var x, y, c;
  // Backward tone mapping
  Func rev_tone_map("rev_tone_map");
    Expr rev_tone_idx = cast<uint8_t>((*in_func)(x,y,c) * 255.0f);
    rev_tone_map(x,y,c) = (*rev_tone_h)(c,rev_tone_idx) ;
  return rev_tone_map;
}

Func make_tone_map( Func *in_func,
                    Image<float> *rev_tone_h ) {
  Var x, y, c;
  // Forward tone mapping
  Func tone_map("tone_map");
    RDom idx2(0,256);
    // Theres a lot in this one line! This line finds the entry in 
    // the reverse tone mapping function which is closest to this Func's
    // input. It then scales back down to the 0-1 range expected as the 
    // output for every stage. Effectively it reverses the reverse 
    // tone mapping function.
    tone_map(x,y,c) = (argmin( abs( (*rev_tone_h)(c,idx2) 
                                 - (*in_func)(x,y,c) ) )[0])/256.0f;
  return tone_map;
}

Func make_pwl_tone_map( Func *in_func ) {
  Var x, y, c;  

  // Approximate forward tone mapping
  Func pwl_tone_map("approx-tonemap");
       pwl_tone_map(x,y,c) = max( min( select(
              (*in_func)(x,y,c) < 32,  (*in_func)(x,y,c) * 4,
              (*in_func)(x,y,c) < 128, (*in_func)(x,y,c) + 96,
                                       (*in_func)(x,y,c)/4 + 192)
                                          , 255), 0);
  return pwl_tone_map;
}

Func make_rbf_ctrl_pts( Func *in_func, 
                        int num_ctrl_pts,
                        Image<float> *ctrl_pts_h, 
                        Image<float> *weights_h ) {
  Var x, y, c;
  // Weighted radial basis function for gamut mapping
  Func rbf_ctrl_pts("rbf_ctrl_pts");
    // Initialization with all zero
    rbf_ctrl_pts(x,y,c) = cast<float>(0);
    // Index to iterate with
    RDom idx(0,num_ctrl_pts);
    // Loop code
    // Subtract the vectors 
    Expr red_sub   = (*in_func)(x,y,0) - (*ctrl_pts_h)(0,idx);
    Expr green_sub = (*in_func)(x,y,1) - (*ctrl_pts_h)(1,idx);
    Expr blue_sub  = (*in_func)(x,y,2) - (*ctrl_pts_h)(2,idx);
    // Take the L2 norm to get the distance
    Expr dist      = sqrt( red_sub*red_sub + 
                              green_sub*green_sub + 
                              blue_sub*blue_sub );
    // Update persistant loop variables
    rbf_ctrl_pts(x,y,c) = select( c == 0, rbf_ctrl_pts(x,y,c) +
                                    ( (*weights_h)(0,idx) * dist),
                                  c == 1, rbf_ctrl_pts(x,y,c) + 
                                    ( (*weights_h)(1,idx) * dist),
                                          rbf_ctrl_pts(x,y,c) + 
                                    ( (*weights_h)(2,idx) * dist));
  return rbf_ctrl_pts;
}

Func make_rbf_biases( Func *in_func, 
                      Func *rbf_ctrl_pts, 
                      vector<vector<float>> *coefs ) {
  Var x, y, c;
  // Add on the biases for the RBF
  Func rbf_biases("rbf_biases");
    rbf_biases(x,y,c) = max( select( 
      c == 0, (*rbf_ctrl_pts)(x,y,0)     + (*coefs)[0][0] + (*coefs)[1][0]*(*in_func)(x,y,0) +
        (*coefs)[2][0]*(*in_func)(x,y,1) + (*coefs)[3][0]*(*in_func)(x,y,2),
      c == 1, (*rbf_ctrl_pts)(x,y,1)     + (*coefs)[0][1] + (*coefs)[1][1]*(*in_func)(x,y,0) +
        (*coefs)[2][1]*(*in_func)(x,y,1) + (*coefs)[3][1]*(*in_func)(x,y,2),
              (*rbf_ctrl_pts)(x,y,2)     + (*coefs)[0][2] + (*coefs)[1][2]*(*in_func)(x,y,0) +
        (*coefs)[2][2]*(*in_func)(x,y,1) + (*coefs)[3][2]*(*in_func)(x,y,2))
                            , 0);
  return rbf_biases;
}

Func make_transform( Func *in_func, 
                     vector<vector<float>> *TsTw_tran ) {
  Var x, y, c;
  // Reverse color map and white balance transform
  Func transform("transform");
    transform(x,y,c) = max( select(
      // Perform matrix multiplication, set min of 0
      c == 0, (*in_func)(x,y,0)*(*TsTw_tran)[0][0]
            + (*in_func)(x,y,1)*(*TsTw_tran)[1][0]
            + (*in_func)(x,y,2)*(*TsTw_tran)[2][0],
      c == 1, (*in_func)(x,y,0)*(*TsTw_tran)[0][1]
            + (*in_func)(x,y,1)*(*TsTw_tran)[1][1]
            + (*in_func)(x,y,2)*(*TsTw_tran)[2][1],
              (*in_func)(x,y,0)*(*TsTw_tran)[0][2]
            + (*in_func)(x,y,1)*(*TsTw_tran)[1][2]
            + (*in_func)(x,y,2)*(*TsTw_tran)[2][2])
                                                      , 0);
  return transform;
}

// Note: This is an implementation of the noise model found in the paper below:
// "Noise measurement for raw-data of digital imaging sensors by 
// automatic segmentation of non-uniform targets"
float get_normal_dist_rand( float mean, float std_dev ) {
  std::default_random_engine generator;
  std::normal_distribution<float> distribution(mean,std_dev);
  float out = distribution(generator);
  return out;
}

Func make_get_std_dev( Func *in_func ) {
  Var x, y, c;
  float q = 0.0060;
  float p = 0.0500;
  // std_dev = q * sqrt(unnoised_pixel - p)
  Func get_std_dev("get_std_dev");
    get_std_dev(x,y,c) = q * sqrt( (*in_func)(x,y,c) - p );
  return get_std_dev;  
}

// Note: This gaussian blur function is adapted from Halide CVPR 2015 code
Image<float> gaussian_blur(Image<float> *in) {

    // Define a 7x7 Gaussian Blur with a repeat-edge boundary condition.
    float sigma = 1.5f;

    Var x, y, c;
    Func kernel;
    kernel(x) = exp(-x*x/(2*sigma*sigma)) / (sqrtf(2*M_PI)*sigma);

    Func in_bounded = BoundaryConditions::repeat_edge(*in);

    Func blur_y;
    blur_y(x, y, c) = (kernel(0) * in_bounded(x, y, c) +
                       kernel(1) * (in_bounded(x, y-1, c) +
                                    in_bounded(x, y+1, c)) +
                       kernel(2) * (in_bounded(x, y-2, c) +
                                    in_bounded(x, y+2, c)) +
                       kernel(3) * (in_bounded(x, y-3, c) +
                                    in_bounded(x, y+3, c)));

    Func blur_x;
    blur_x(x, y, c) = (kernel(0) * blur_y(x, y, c) +
                       kernel(1) * (blur_y(x-1, y, c) +
                                    blur_y(x+1, y, c)) +
                       kernel(2) * (blur_y(x-2, y, c) +
                                    blur_y(x+2, y, c)) +
                       kernel(3) * (blur_y(x-3, y, c) +
                                    blur_y(x+3, y, c)));

    // Schedule it.
    blur_x.compute_root().vectorize(x, 8).parallel(y);
    blur_y.compute_at(blur_x, y).vectorize(x, 8);
 
    // Realize the pipeline
    Image<float> output((*in).width(),
                        (*in).height(),
                        (*in).channels());
    blur_x.realize(output);

    return output;
}

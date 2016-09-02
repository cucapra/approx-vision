#include "MatrixOps.h"

using namespace std;

// Transpose a matrix
vector<vector<float>> transpose_mat (vector<vector<float>> inmat) {

  // Get size of input
  int in_width  = inmat[0].size();
  int in_height = inmat.size();

  // Define vectors
  vector<vector<float>> outmat (in_width, vector<float>(in_height));

  // Transpose the matrix
  for (int i=0; i<in_height; i++) {
    for (int j=0; j<in_width; j++) {
      outmat[j][i] = inmat[i][j];
    }
  }

  return outmat;

}

// Matrix Matrix Dot Product
vector<vector<float>> dot_matmat (vector<vector<float>> a, vector<vector<float>> b) {

  int width  = b[0].size();
  int height = a.size();

  int a_inner_dim = a[0].size();
  int b_inner_dim = b.size();

  // Define output size
  vector<vector<float>> result (height, vector<float>(width));

  // Take the transpose of b for more convenient multiplication
  vector<vector<float>> b_trans = transpose_mat(b); 

  // Verify that the inner dimensions are the same
  if (a_inner_dim != b_inner_dim)
    throw invalid_argument("Attempt to dot product matrices with different inner dimensions");
   
  // Multiply the matrices
  for (int i=0; i<height; i++) {
    for (int j=0; j<width; j++) {
      result[i][j] = dot_vecvec(a[i],b_trans[j]);
    }
  }

  return result;

}

// Vector Matrix Dot Product
vector<float> dot_vecmat (vector<float> a, vector<vector<float>> b) {

}

// Vector Vector Dot Product
float dot_vecvec (vector<float> a, vector<float> b) {

  float result = 0;
  int   size_a = a.size();
  int   size_b = b.size();

  // Verify that both vectors are of the same size
  if (size_a != size_b)
    throw invalid_argument("Attempt to dot product vectors of different size");
  
  // Multiply the vectors
  for (int i=0; i<size_a; i++) {
    result += a[i] * b[i];
  }

  return result;
}


// Three by three matrix inversion
vector<vector<float>> inv_3x3mat (vector<vector<float>> inmat) {
  vector<vector<float>> result;
  result = inmat;

  float det = inmat[0][0] * (inmat[1][1] * inmat[2][2] - inmat[2][1] * inmat[1][2]) -
              inmat[0][1] * (inmat[1][0] * inmat[2][2] - inmat[1][2] * inmat[2][0]) +
              inmat[0][2] * (inmat[1][0] * inmat[2][1] - inmat[1][1] * inmat[2][0]);

  float invdet = 1/det;

  result[0][0] = (inmat[1][1] * inmat[2][2] - inmat[2][1] * inmat[1][2]) * invdet;
  result[0][1] = (inmat[0][2] * inmat[2][1] - inmat[0][1] * inmat[2][2]) * invdet;
  result[0][2] = (inmat[0][1] * inmat[1][2] - inmat[0][2] * inmat[1][1]) * invdet;
  result[1][0] = (inmat[1][2] * inmat[2][0] - inmat[1][0] * inmat[2][2]) * invdet;
  result[1][1] = (inmat[0][0] * inmat[2][2] - inmat[0][2] * inmat[2][0]) * invdet;
  result[1][2] = (inmat[1][0] * inmat[0][2] - inmat[0][0] * inmat[1][2]) * invdet;
  result[2][0] = (inmat[1][0] * inmat[2][1] - inmat[2][0] * inmat[1][1]) * invdet;
  result[2][1] = (inmat[2][0] * inmat[0][1] - inmat[0][0] * inmat[2][1]) * invdet;
  result[2][2] = (inmat[0][0] * inmat[1][1] - inmat[1][0] * inmat[0][1]) * invdet;

  return result;
}

// Display matrix
void disp_mat (vector<vector<float>> mat) {
  
  int height = mat.size();
  int width  = mat[0].size();

  for (int i=0; i<height; i++) {
    for (int j=0; j<width; j++) {
      printf("%f,",mat[i][j]);
    }
    printf("\n");
  }
  printf("\n");

}

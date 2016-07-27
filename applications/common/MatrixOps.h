#include <vector>
#include <stdexcept>

using namespace std;

// Transpose a matrix
vector<vector<float>> transpose_mat (vector<vector<float>> inmat);

// Matrix Matrix Dot Product
vector<vector<float>> dot_matmat (vector<vector<float>> a, vector<vector<float>> b);

// Vector Matrix Dot Product
vector<float> dot_vecmat (vector<float> a, vector<vector<float>> b);

// Vector Vector Dot Product
float dot_vecvec (vector<float> a, vector<float> b);

// Three by three matrix inversion
vector<vector<float>> inv_3x3mat (vector<vector<float>> inmat);

// Visualize matrix
void disp_mat (vector<vector<float>> mat);

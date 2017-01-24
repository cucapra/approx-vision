#! /usr/bin/env python

import matplotlib.pyplot as plt
import sys
import numpy as np
from numpy import sqrt, pi, exp, linspace
from scipy.optimize import curve_fit
from scipy.interpolate import interp1d
from scipy.interpolate import Rbf, InterpolatedUnivariateSpline

# Read in v6 data as v6 has skipped all but demosaicing
# (Full raw data but without zeros due to bayer pattern)
#filename = '/home/mbuckler/datasets/cifar-10/v6/data_batch_1.bin'
filename = '/home/mark/Dropbox/datasets/cifar-10/v6/data_batch_1.bin'


# Tone mapping function unecessary as we have read in v6 data 
'''
#########################################
# Read in and interpolate tone mapping
camf_r = []
camf_g = []
camf_b = []
camf_avg = []

firstline = True                                                         
for line in open("../cam_models/NikonD7000/jpg2raw_respFcns.txt"):
  listvals = line.split(" ")                                             
  if firstline == False:                                                 
    camf_r.append(float(listvals[0])*255.0)                         
    camf_g.append(float(listvals[1])*255.0)                         
    camf_b.append(float(listvals[2])*255.0)                        
    camf_avg.append( ( float(listvals[0]) + 
                       float(listvals[1]) +
                       float(listvals[2]) ) * (255.0/3) )
  firstline = False 

camf_cubic = interp1d(range(0,256),camf_avg,kind='cubic')
'''

#########################################
# Generate PDFs

# Establish empty histograms
histogram_r = np.zeros(256)
histogram_g = np.zeros(256)
histogram_b = np.zeros(256)
histogram_avg = np.zeros(256)
total       = 0
pixel_data  = []
num_samps   = 0

with open(filename, 'rb') as f:
  for i in range(0,1000): # 10000
    # Read in the image label
    byte_s = f.read(1)

    for c in range(0,3):
      for y in range(0,32):
        for x in range(0,32):
          byte_s      = f.read(1)
          hist_idx    = ord(byte_s[0])
          #hist_idx    = byte_s[0]
          pixel_data.append(hist_idx)
          num_samps += 1
          if c == 0:
            histogram_r[hist_idx] += 1
            histogram_avg[hist_idx] += 1
            total += 1
          if c == 1:
            histogram_g[hist_idx] += 1
            # Double count green
            histogram_avg[hist_idx] += 2 
            total += 2
          if c == 2:
            histogram_b[hist_idx] += 1
            histogram_avg[hist_idx] += 1
            total += 1

# Normalize the histograms
histogram_r = histogram_r / (total/4)
histogram_g = histogram_g / (total/4)
histogram_b = histogram_b / (total/4)
histogram_avg = histogram_avg / total

def gaussian(x, amp, cen, wid):
    return amp * exp(-(x-cen)**2 / wid)


init_vals = [0.3, 15, 5]

best_vals, covar = curve_fit(gaussian, range(0,256), histogram_avg,
        p0=init_vals)

print (best_vals)

PDF_cubic   = interp1d(range(0,256),histogram_avg,kind='cubic')
PDF_rbf = Rbf(range(0,256),histogram_avg)

avg_PDF_gauss = []
avg_PDF_cubic = []
avg_PDF_rbf   = []

for x in range(0,256):
  avg_PDF_gauss.append(
    gaussian(x,best_vals[0],best_vals[1],best_vals[2]))
  avg_PDF_cubic.append( PDF_cubic(x) )
  avg_PDF_rbf.append( PDF_rbf(x) )

'''

plt.figure(2)

plt.subplot(411)
plt.plot(range(0,256),histogram_avg,'b',
         range(0,256),avg_PDF_cubic, 'k')
plt.axis([0,255,0,1])

plt.subplot(412)
plt.plot(range(0,256),histogram_r,'b',
         range(0,256),avg_PDF_cubic, 'k')
plt.axis([0,255,0,1])

plt.subplot(413)
plt.plot(range(0,256),histogram_g,'b',
         range(0,256),avg_PDF_cubic, 'k')
plt.axis([0,255,0,1])

plt.subplot(414)
plt.plot(range(0,256),histogram_b,'b',
         range(0,256),avg_PDF_cubic, 'k')
plt.axis([0,255,0,1])

plt.show()

'''

# Inverse sampling technique adapted:
# http://www.nehalemlabs.net/prototype/blog/2013/12/16/how-to-do-inverse-transformation-sampling-in-scipy-and-numpy/

import numpy as np
import scipy.interpolate as interpolate

def inverse_transform_sampling(data, n_bins=256, n_samples=1000):
        hist, bin_edges = np.histogram(data, bins=n_bins, density=True)
        cum_values = np.zeros(bin_edges.shape)
        cum_values[1:] = np.cumsum(hist*np.diff(bin_edges))
        inv_cdf = interpolate.interp1d(cum_values,
        bin_edges)
        r = np.random.rand(n_samples)
        return inv_cdf(r)

cdf_rand_samples = inverse_transform_sampling(pixel_data,256,100000)


'''
plt.figure(5)
plt.subplot(2,1,1)
plt.hist(pixel_data,256,normed=1)
plt.xlim([0,255])
plt.subplot(2,1,2)
plt.hist(cdf_rand_samples,256,normed=1)
plt.xlim([0,255])
plt.show()
'''

###################################
# Define functions for quantization experiments

# First, we define a function which will calculate the RMSE
def get_rmse(predictions, targets):
    return np.sqrt(((predictions - targets) ** 2).mean())

# Convert rmse to psnr
# Note that the maximum value used in the conversion is the 
# maximum random sample, as error evaluations are made against
# the original continuous samples
def rmse2psnr(rmse):
    return 20.0*np.math.log((255/rmse),10)

# Here we define a linear quantizer with a configurable
# number of bits
def encode_linear(samples,num_bits):
    # Largest sample in the data
    max_sample     = max(samples)
    # Largest possible value that can be used to encode
    max_encode     = (2**num_bits)-1
    # Scale samples so that their maximum is the largest
    # possible encoding value
    scaled_samples = [val*(max_encode/max_sample) for val in samples]
    # Round to closest possible encoding value (integer)
    encoded        = [round(val,0) for val in scaled_samples]
    return max_sample, encoded

# And here we have the accompanying decoder for the linear
# quantizer
def decode_linear(encoded,num_bits,max_sample):
    # Largest possible value that could have been used to decode
    max_encode     = (2**num_bits)-1
    # Re-scale to decode
    decoded   = [val * ( max_sample/max_encode) for val in encoded]
    return decoded

# Here we define a linear quantizer with a configurable
# number of bits
def encode_logarithmic(samples,num_bits):
    # Convert the samples to the logarithmic space
    samples = [np.log(val) for val in samples]
    # Largest sample in the data
    max_sample     = max(samples)
    # Largest possible value that can be used to encode
    max_encode     = (2**num_bits)-1
    # Scale samples so that their maximum is the largest
    # possible encoding value
    scaled_samples = [val*(max_encode/max_sample) for val in samples]
    # Round to closest possible encoding value (integer)
    encoded        = [round(val,0) for val in scaled_samples]
    return max_sample, encoded

# And here we have the accompanying decoder for the logarithmic
# quantizer
def decode_logarithmic(encoded,num_bits,max_sample):
    # Largest possible value that could have been used to decode
    max_encode     = (2**num_bits)-1
    # Re-scale to decode
    decoded   = [val * ( max_sample/max_encode) for val in encoded]
    # Finish by reversing the logarithmic encoding to linearize
    decoded   = [np.exp(val) for val in decoded]
    return decoded

###################################
# Linear quantization analysis

# 12 bit linear
max_sample, encoded_12bit = encode_linear(cdf_rand_samples,12)
decoded_12bit = decode_linear(encoded_12bit,12,max_sample)
rmse = get_rmse(decoded_12bit,cdf_rand_samples)
print 'Linearly encoded raw 12 bit data'
print '  RMSE: '+str(rmse)
print '  PSNR: '+str(rmse2psnr(rmse))+' dB'

# 8 bit linear
max_sample, encoded_8bit = encode_linear(cdf_rand_samples,8)
decoded_8bit = decode_linear(encoded_8bit,8,max_sample)
rmse = get_rmse(decoded_8bit,cdf_rand_samples)
print 'Linearly encoded raw 8 bit data'
print '  RMSE: '+str(rmse)
print '  PSNR: '+str(rmse2psnr(rmse))+' dB'

###################################
# Logarithmic quantization analysis

###################################
# Lloyd-max quantization analyis

import scipy.integrate as integrate

max_sample = max(cdf_rand_samples)

'''
def lognorm_func (in_):
    return (np.exp(-(np.log(in_) - mu)**2 / (2 * sigma**2))
        / (in_ * sigma * np.sqrt(2 * np.pi)))
def scaled_lognorm_func (in_):
    return in_ * (np.exp(-(np.log(in_) - mu)**2 / (2 * sigma**2))
        / (in_ * sigma * np.sqrt(2 * np.pi)))
'''

def pdf_func (in_):
    return PDF_cubic(in_)
def scaled_pdf_func (in_):
    return in_ * PDF_cubic(in_)


# Represent with 8 bits
num_bits = 8

# The number of representation points
M = (2**num_bits)

# Initialize a and b
lloydmax_a = [None] * M
lloydmax_b = [None] * M
init_step_size = max_sample/M
for i in range(0,M):
    lloydmax_b[i] = (i+1)*init_step_size
for i in range(0,M):
    lloydmax_a[i] = lloydmax_b[i] - (init_step_size/2)

def lloydmax_encode_single (in_):
    idx  = 0
    while ( in_ > lloydmax_b[idx] ):
        idx = idx + 1
        if idx == M-1:
            break
    return idx

def lloydmax_encode (in_):        
    return [lloydmax_encode_single(val) for val in in_]
    
def lloydmax_decode (in_):
    return [lloydmax_a[val] for val in in_]
    
encoded_lloydmax = lloydmax_encode(cdf_rand_samples)
decoded_lloydmax = lloydmax_decode(encoded_lloydmax)
rmse = get_rmse(decoded_lloydmax,cdf_rand_samples)
print 'Test of Lloydmax initializer'
print rmse
    
def lloydmax_learn (rmse_goal):
    rmse = 1.0
    while (rmse > rmse_goal):
        # Re-initialize b
        for idx in range(0,M-1):
            lloydmax_b[idx] = 0.5*(lloydmax_a[idx+1]+lloydmax_a[idx])
        # Re-initialize a
        for idx in range(0,M):
            if idx == 0:
                integrate_low_end = 0
            else:
                integrate_low_end = lloydmax_b[idx-1]
            integrate_high_end = lloydmax_b[idx]
            
            numerator,err   = integrate.quad(scaled_pdf_func,
                                             integrate_low_end,
                                             integrate_high_end)
            denominator,err = integrate.quad(pdf_func,
                                             integrate_low_end,
                                             integrate_high_end)
               
            lloydmax_a[idx] = numerator/denominator
        #print lloydmax_a[100]
        # Test the current RMSE
        encoded_lloydmax = lloydmax_encode(cdf_rand_samples)
        decoded_lloydmax = lloydmax_decode(encoded_lloydmax)
        rmse = get_rmse(decoded_lloydmax,cdf_rand_samples)
        #print np.mean(lloydmax_b)
        print '  RMSE: '+str(rmse)
        print '  PSNR: '+str(rmse2psnr(rmse))+' dB'
        #print '\n'
    
lloydmax_learn(0.15)
print 'success!'










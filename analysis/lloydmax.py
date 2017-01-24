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

cdf_rand_samples = inverse_transform_sampling(pixel_data,256,2000)

plt.figure(5)
plt.subplot(2,1,1)
plt.hist(pixel_data,256,normed=1)
plt.xlim([0,255])
plt.subplot(2,1,2)
plt.hist(cdf_rand_samples,256,normed=1)
plt.xlim([0,255])
plt.show()




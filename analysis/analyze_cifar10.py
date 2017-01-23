#! /usr/bin/env python

import matplotlib.pyplot as plt
import sys
import numpy as np
from numpy import sqrt, pi, exp, linspace

# Only argument is path to cifar-10 data file
filename = sys.argv[1]

# Establish empty histograms
histogram_r = np.zeros(256)
histogram_g = np.zeros(256)
histogram_b = np.zeros(256)

with open(filename, 'rb') as f:
  for i in range(0,100): # 10000
    # Read in the image label
    byte_s = f.read(1)

    for c in range(0,3):
      for y in range(0,32):
        for x in range(0,32):
          byte_s      = f.read(1)
          hist_idx    = ord(byte_s[0])
          if c == 0:
            histogram_r[hist_idx] += 1
          if c == 1:
            histogram_g[hist_idx] += 1
          if c == 2:
            histogram_b[hist_idx] += 1

# Normalize the histograms
histogram_r = histogram_r / max(histogram_r)
histogram_g = histogram_g / max(histogram_g)
histogram_b = histogram_b / max(histogram_b)

plt.figure(1)
plt.subplot(311)
plt.bar(range(0,256),histogram_r,width=0.5,color='r')
plt.axis([0,255,0,1])

plt.subplot(312)
plt.bar(range(0,256),histogram_g,width=0.5,color='g')
plt.axis([0,255,0,1])

plt.subplot(313)
plt.bar(range(0,256),histogram_b,width=0.5,color='b')
plt.axis([0,255,0,1])


#plt.show()

def gaussian(x, amp, cen, wid):
    return amp * exp(-(x-cen)**2 / wid)

from scipy.optimize import curve_fit

init_vals = [0.8, 125, 50]

best_vals, covar = curve_fit(gaussian, range(0,256), histogram_g,
        p0=init_vals)

print best_vals

print len(range(0,256))
print len((gaussian(x,best_vals[0],best_vals[1],best_vals[2])            
            for x in range(0,256)))

r_gauss = []
g_gauss = []
b_gauss = []
for intensity in range(0,256):
    r_gauss = 


'''
plt.figure(2)
plt.plot((range(0,256)), \
         (gaussian(x,best_vals[0],best_vals[1],best_vals[2]) \
         for x in range(0,256)) )
plt.show()
'''






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
histogram_avg = np.zeros(256)

with open(filename, 'rb') as f:
  for i in range(0,10000): # 10000
    # Read in the image label
    byte_s = f.read(1)

    for c in range(0,3):
      for y in range(0,32):
        for x in range(0,32):
          byte_s      = f.read(1)
          hist_idx    = byte_s[0] #ord(byte_s[0])
          if c == 0:
            histogram_r[hist_idx] += 1
            histogram_avg[hist_idx] += 1
          if c == 1:
            histogram_g[hist_idx] += 1
            histogram_avg[hist_idx] += 1
          if c == 2:
            histogram_b[hist_idx] += 1
            histogram_avg[hist_idx] += 1

# Normalize the histograms
histogram_r = histogram_r / max(histogram_r)
histogram_g = histogram_g / max(histogram_g)
histogram_b = histogram_b / max(histogram_b)
histogram_avg = histogram_avg / max(histogram_avg)

def gaussian(x, amp, cen, wid):
    return amp * exp(-(x-cen)**2 / wid)

from scipy.optimize import curve_fit

init_vals = [0.8, 125, 50]

best_vals, covar = curve_fit(gaussian, range(0,256), histogram_avg,
        p0=init_vals)

print (best_vals)

from scipy.interpolate import interp1d
from scipy.interpolate import Rbf, InterpolatedUnivariateSpline

f   = interp1d(range(0,256),histogram_avg,kind='cubic')
rbf = Rbf(range(0,256),histogram_avg)

avg_gauss = []

avg_cubic = []

avg_rbf   = []

for x in range(0,256):
  avg_gauss.append(
    gaussian(x,best_vals[0],best_vals[1],best_vals[2]))
  avg_cubic.append( f(x) )
  avg_rbf.append( rbf(x) )

plt.figure(1)

plt.subplot(411)
plt.plot(range(0,256),histogram_avg,'b',
         range(0,256),avg_gauss, 'g',
         range(0,256),avg_rbf, 'k')
plt.axis([0,255,0,1])

plt.subplot(412)
plt.plot(range(0,256),histogram_r,'b',
         range(0,256),avg_gauss, 'g',
         range(0,256),avg_rbf, 'k')
plt.axis([0,255,0,1])

plt.subplot(413)
plt.plot(range(0,256),histogram_g,'b',
         range(0,256),avg_gauss, 'g',
         range(0,256),avg_rbf, 'k')
plt.axis([0,255,0,1])

plt.subplot(414)
plt.plot(range(0,256),histogram_b,'b',
         range(0,256),avg_gauss, 'g',
         range(0,256),avg_rbf, 'k')
plt.axis([0,255,0,1])



plt.show()







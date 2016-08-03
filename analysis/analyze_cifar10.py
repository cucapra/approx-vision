#! /usr/bin/env python

import matplotlib.pyplot as plt
import sys
import numpy as np

# Only argument is path to cifar-10 data file
filename = sys.argv[1]

# Establish empty histogram
histogram = np.zeros(16)

# Initialize sum as zero
total_sum = 0

with open(filename, 'rb') as f:
  for i in range(0,10000):
    # Read in the image label
    byte_s = f.read(1)

    # Initialize sum as zero
    img_sum = 0

    for c in range(0,3):
      for y in range(0,32):
        for x in range(0,32):
          byte_s      = f.read(1)
          channel_val = float(ord(byte_s[0]))
          img_sum    += channel_val
          hist_idx    = int(channel_val/16)
          histogram[hist_idx] += 1

    img_avg = img_sum / float(3*32*32)

    total_sum = total_sum + img_avg

total_avg = total_sum / float(10000)

print('Total channel average:')
print(total_avg)

# Normalize the histogram
histogram = histogram / max(histogram)

plt.bar(range(16),histogram)

plt.show()



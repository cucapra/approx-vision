# Script for generating linear levels
#
# Use:
# python gen_lin_levels.py <num_bits>
#

import sys

num_bits = int(sys.argv[1])

f = open('lin_levels_'+str(num_bits)+'.txt', 'w')

for i in range(int(2**num_bits)):
  print>>f, str(float(i+1)/float(2**num_bits))

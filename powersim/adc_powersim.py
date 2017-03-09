# ADC Power Sim
#
# Script for generating ADC energy estimations. This script establishes
# the minimum capacitor size as the unifying element between ADC
# designs. This script assumes an SAR ADC but does not assume ADC
# linearity.
# 
# The only input that the script needs is a txt file containing a line
# corresponding to each fractional level associated with each
# encoded value. This means that for an n bit ADC the txt file should
# have 2^n entries and the final entry will have a value of 1.0
# 
# Use:
# python adc_powersim.py <adc_levels_file> <input_PDF_file>
# 
# Note that ../analysis/cifar_10_PDF.txt contains the PDF for the input
# for natural images

import numpy as np
import sys

levels_file = sys.argv[1] 
pdf_file    = sys.argv[2]

levels_volt = []
levels_energy = []
n = 0

# Quantization function
def quantize( voltage ):

  # Initialize energy counter
  energy = 0
  
  # Run through the quantization process, keeping track of any charged
  # capacitance. Use binary search (as in SAR ADCs), so start
  # mid-way through the range
  encoded_val = ((2**n)/2)-1
  energy += levels_volt[encoded_val] - 0
  for i in range(1,n+1):
    # If the last bit
    if (i == n):
      # Only change if need to increment
      if (voltage > levels_volt[encoded_val]):
        # No energy change since no new level after this
        encoded_val += 1
    else:
      if (voltage <= levels_volt[encoded_val]):
        # No energy change since we are discharging caps rather than
        # charging caps (reducing voltage)
        encoded_val -= (2**n)/(2**(i+1))
      else:
        # Increment energy based on positive change in voltage
        prev_encoded_val = encoded_val
        encoded_val += (2**n)/(2**(i+1)) 
        energy+=(levels_volt[encoded_val]-levels_volt[prev_encoded_val])

  return (encoded_val,energy)


# Load in levels proportional to the full range
with open(levels_file) as f:
  levels_frac = f.readlines()
levels_frac = [float(x.strip('\n')) for x in levels_frac] 


# Compute the number of bits from the number of levels
n = int(np.log2(len(levels_frac)))


# Determine the ratio of smallest capacitor to fractional range
min_c_val = levels_frac[0]

# Establish voltage proportional levels by normalizing to the smallest
# capacitor
for level in levels_frac:
  levels_volt.append( level / min_c_val )

# Compute energy expended to evaluate each of the levels
for l,level_volt in enumerate(levels_volt):
  # Set voltage to encode as equal to the voltage associated with this
  # level
  voltage = level_volt

  # Quantize the value
  encoded_val, level_energy = quantize(voltage)

  # Record the energy it took to quantize
  levels_energy.append(level_energy)

# Print average energy assuming uniform PDF input
#print (sum(levels_energy)/len(levels_energy))

# Read in our PDF
with open(pdf_file) as f2:
  in_data_PDF = f2.readlines()
  in_data_PDF = [float(x.strip('\n')) for x in in_data_PDF] 

# If there are more quantization levels than PDF points (12 bit ADC even
# though our PDF only has 8 bits)
while( len(levels_energy) > len(in_data_PDF) ):
  # Combine quantization level energy scores until the number of levels
  # matches the PDF entries
  even_levels = levels_energy[1::2]
  odd_levels  = levels_energy[::2]
  levels_energy = [x+y for x,y in zip(even_levels, odd_levels)]

# If there are more PDF points than quantization levels (5 bit ADC even
# though our PDF has 8 bits)
while( len(levels_energy) < len(in_data_PDF) ):
  even_PDF = in_data_PDF[1::2]
  odd_PDF  = in_data_PDF[::2]
  in_data_PDF = [x+y for x,y in zip(even_PDF, odd_PDF)]

# Compute the expected value of the energy cost per ADC call given the
# energy cost and value probability
print(sum([x*y for x,y in zip(levels_energy,in_data_PDF)]))

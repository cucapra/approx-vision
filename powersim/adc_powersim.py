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
# python adc_powersim.py <levels_file>
#

import numpy as np
import sys

levels_file = sys.argv[1] 

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
  #print voltage
  #print encoded_val
  #print level_energy
  #print "-----"

  # Record the energy it took to quantize
  levels_energy.append(level_energy)

# Print average energy
print (sum(levels_energy)/len(levels_energy))

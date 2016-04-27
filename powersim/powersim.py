###############################################
# Power Simulator 
#
# Script to compute the energy and power 
# consumption of CMOS image sensors.
###############################################


###############################################
# Parameters

width  = 1000.0  # Pixels
height = 1000.0  # Pixels
vdd    = 2.5     # Supply voltage (V)
f_rate = 40.0    # Frame rate (frames/second)
cmuxw  = 1.0     # Width of column readout multiplexer
read_p = 100.0   # Read percentage (%)
ccurr  = 1e-6    # Current per current mirror (Amps)
pc_sw  = 100e-15 # Pixel switch capacitance (F)
adc_e  = 100e-12 # Energy per adc sample (J)
# https://converterpassion.wordpress.com/2013/06/08/adc-energy-efficiency-trends/


###############################################
# Evaluation

# DC power (current mirrors)
#  pow = V   * I
dc_pow = vdd * ccurr           # Power per read circuit (curr mirror)
dc_pow = dc_pow * width        # Power if every column if every column has a readout circuit
dc_pow = dc_pow / cmuxw        # Total power for all readout circuits, if all values read
dc_pow = dc_pow * (read_p/100) # Total power after current mirror power gating applied

# AC power (line toggling)
#  pow = f      * c_sw  * Vdd^2
ac_pow = f_rate * pc_sw * (vdd**2) # Switching power per pixel
ac_pow = ac_pow * width * height   # Total switching power
# Magnitude expected to be small, so no power gating is applied

# ADC power (conversion)
#   pow = energy / time
adc_pow = adc_e / (1/f_rate)       # Power assuming a single pixel is read
adc_pow = adc_pow * width * height # Total power assuming that all pixels in array are read
adc_pow = adc_pow * (read_p/100)   # Total power after selective reading is applied


###############################################
# Results

# Total power
tot_pow = dc_pow + ac_pow + adc_pow
# Energy per frame
e_per_f = tot_pow * (1/f_rate)

print("")
print("Current mirror power consumption")
print(dc_pow)
print("Switching power consumption")
print(ac_pow)
print("ADC power consumption")
print(adc_pow)
print("")
print("Total power")
print(tot_pow)
print("Energy per frame")
print(e_per_f)
print("")


#! /usr/bin/env python

import subprocess
import sys
from os import listdir
from os.path import isfile, join

if (len(sys.argv) < 2):
  print("Error: Too few arguments. Did you remember to give a pipeline version number?")
  exit()

# Version of pipeline to run
version       = sys.argv[1]

# Directory with virgin cifar data files
data_in_dir   = '../data/cifar10/'

# Directory to put the converted files
data_out_dir  = '../data/cifar10/converted-V'+str(version)+'/'

# Data batch names
data_names    = ['test_batch',
  'data_batch_1',
  'data_batch_2',
  'data_batch_3',
  'data_batch_4',
  'data_batch_5']

# Create the directory for the converted files
subprocess.call(["mkdir",data_out_dir])

# Compile the converter
subprocess.call(["make",('V'+str(version))])

# For each batch
for x in range(0,6):
  # Start a script to process a batch of the images
  pid = subprocess.Popen(["./convert-V"+str(version),
                          data_in_dir+data_names[x]+".bin",
                          data_out_dir+data_names[x]+"_converted_V"+str(version)+".bin",
                         ])

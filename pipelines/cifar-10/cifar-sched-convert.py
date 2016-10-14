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
data_in_dir   = '~/datasets/cifar-10/v0/'

# Directory to put the converted files
data_out_dir  = '~/datasets/cifar-10/v'+str(version)+'/'

# Data batch names
data_names    = ['test_batch.bin',
  'data_batch_1.bin',
  'data_batch_2.bin',
  'data_batch_3.bin',
  'data_batch_4.bin',
  'data_batch_5.bin']

# Create the directory for the converted files
subprocess.call(["mkdir",data_out_dir])

# Compile the converter
subprocess.call(["make",('version='+str(version))])

# For each batch
for x in range(0,6):
  # Start a script to process a batch of the images
  pid = subprocess.Popen(["./pipeline_V"+str(version),
                          data_in_dir+data_names[x],
                          data_out_dir+data_names[x],
                         ])

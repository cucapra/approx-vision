#! /usr/bin/env python

import subprocess
import sys
from os import listdir
from os.path import isfile, join

if (len(sys.argv) < 3):
  print("Usage: \n python cifar-sched-convert.py "+
    "<input_pipe_vers_#> <convert_with_pipe_vers_#>")
  exit()

# Version of pipeline to run
in_version    = sys.argv[1]
version       = sys.argv[2]

# Directory with virgin cifar data files
data_in_dir   = '/datasets/cifar-10/v'+str(in_version)+'/'

# Directory to put the converted files
data_out_dir  = '/datasets/cifar-10/v'+str(version)+'/'

# Data batch names
data_names    = ['test_batch.bin',
  'data_batch_1.bin',
  'data_batch_2.bin',
  'data_batch_3.bin',
  'data_batch_4.bin',
  'data_batch_5.bin']

# Create the directory for the converted files
subprocess.call(["mkdir",data_out_dir])

# Copy the metadata (unchanged)
subprocess.call(["cp",data_in_dir+"batches.meta.txt",data_out_dir])

# Compile the converter
subprocess.call(["make",('version='+str(version))])

# For each batch
for x in range(0,6):
  # Start a script to process a batch of the images
  pid = subprocess.Popen(["./pipeline_V"+str(version)+'.o',
                          data_in_dir+data_names[x],
                          data_out_dir+data_names[x],
                         ])

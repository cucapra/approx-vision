#! /usr/bin/env python

import subprocess
import sys
from os import listdir
from os.path import isfile, join
import psutil


num_threads = 2
datasetpath = '/datasets/lfw/'
vers_to_run = [ 1]
in_vers     = [ 0]

for i,version in enumerate(vers_to_run):

  inputpath   = datasetpath + 'v' + str(in_vers[i]) + '/'
  outputpath  = datasetpath + 'v' + str(version)+     '/'

  # Make the directory for this section  
  subprocess.call(['mkdir',out_img_dir])

  # Make a list of directories
  dir_list = [f for f in listdir(inputpath)]
  dir_list.sort()
 
  # Get number of directories
  num_dirs = len(dir_list)

  # Initialize bounds for running
  dirs_per_thread = num_dirs / num_dirs
  start_dir_id    = 0
  end_dir_id      = start_dir_id + dirs_per+thread - 1

  # Establish empty set of processes
  procs = []

  # For every thread
  for x in range(0,num_threads):
    # In case the number of images is not a multiple of the number of threads
    if (num_imgs-start_img_id <= 2*imgs_per_thread):
      end_img_id = num_imgs - 1

    # Start a script to process a batch of the images
    procs.append( subprocess.Popen(["python thread_sched.py",
				    str(version),
				    str(start_dir_id),
				    str(end_dir_id),
				    inputpath,
				    outputpath,
				    str(x)
                                   ]) )

    # Update the bounds for the next script
    start_img_id = start_img_id + imgs_per_thread
    end_img_id   = end_img_id   + imgs_per_thread

    # Initialize the number of procs
    num_busy_procs = num_threads

    # Busy wait
    while(num_busy_procs != 0):
      ls = [psutil.Process(proc.pid) for proc in procs]







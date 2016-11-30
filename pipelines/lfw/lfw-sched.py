#! /usr/bin/env python

import subprocess
import sys
from subprocess import call
import os
from os import listdir
from os.path import isfile, join
import psutil
import time

num_threads = 2
datasetpath = '/datasets/lfw/'

vers_to_run = [ 1, 2]
in_vers     = [ 0, 0]

for i,version in enumerate(vers_to_run):

  # Compile the new version to run
  call('make --directory ../common/ version='+str(version),shell=True)

  inputpath   = datasetpath + 'v' + str(in_vers[i]) + '/'
  outputpath  = datasetpath + 'v' + str(version)+     '/'

  # Make the directory for this section  
  subprocess.call(['mkdir',outputpath])

  # Make a list of directories
  dir_list = [f for f in listdir(inputpath)]
  dir_list.sort()
 
  # Get number of directories
  num_dirs = len(dir_list)

  # Initialize bounds for running
  dirs_per_thread = num_dirs / num_threads
  start_dir_id    = 0
  end_dir_id      = start_dir_id + dirs_per_thread - 1

  # Establish empty set of processes
  procs = []

  # Spawn all threads
  for x in range(0,num_threads):
    # In case the number of images is not a multiple of the number of threads
    if (num_dirs-start_dir_id <= 2*dirs_per_thread):
      end_dir_id = num_dirs - 1

    # Start a script to process a batch of the images
    procs.append( subprocess.Popen("python thread-sched.py "+
            str(version)+' '+
            str(start_dir_id)+' '+
            str(end_dir_id)+' '+
            inputpath+' '+
            outputpath+' '+
            str(x)+' '
                                   ,shell=True) )

    # Update the bounds for the next script
    start_dir_id = start_dir_id + dirs_per_thread
    end_dir_id   = end_dir_id   + dirs_per_thread

  proc_states = [True] * num_threads

  # Check every minute to see if all threads have finished
  while((not any(proc_states)) != True):
    # Previous check to see if processes have completed
    proc_states = [os.path.exists("/proc/"+str(proc.pid)) for proc in procs]

    time.sleep(5)

  procs[:] = []



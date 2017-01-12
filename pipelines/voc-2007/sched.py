#! /usr/bin/env python

import subprocess
import sys
from os import listdir
from os.path import isfile, join
import psutil
import time

vers_to_run = [ 1, 2, 6,13,41,65,67,69,71]
in_vers     = [ 0, 0, 0, 0, 0,13,13,13,13]

num_threads = 12

# The directory to convert
datasetpath   = '/datasets/voc-2007/'

for i, version in enumerate(vers_to_run):

  in_version = in_vers[i]

  # Copy all but the JPEG images
  subprocess.call('rsync -av '+
                   datasetpath+'/v'+str(in_version)+'/ '+
                   datasetpath+'/v'+str(version)+' '+
                   '--exclude VOC2007/JPEGImages',
                   shell=True)

  in_img_dir  = datasetpath+'v'+str(in_version)+'/VOC2007/JPEGImages/'
  out_img_dir = datasetpath+'v'+str(version)+'/VOC2007/JPEGImages/'

  # Make the directory for this section  
  subprocess.call('mkdir '+out_img_dir,shell=True)
  
  # Get list of files in directory
  file_list = [f for f in listdir(in_img_dir) if 
                                          isfile(join(in_img_dir, f))]
  file_list.sort()

  # Number of images in the directory
  num_imgs = len(file_list)

  # Initialize bounds for running the pipe
  imgs_per_thread = num_imgs / num_threads
  start_img_id = 0
  end_img_id   = start_img_id + imgs_per_thread - 1

  # Establish empty set of processes
  procs = []

  # For every split
  for x in range(0,num_threads):
    # Provide a temporary folder for this script
    subprocess.call(['mkdir',in_img_dir+'/temp'+str(x)])

    # In case the number of images is not a multiple of the number of threads
    if (num_imgs-start_img_id <= 2*imgs_per_thread):
      end_img_id = num_imgs - 1

    # Start a script to process a subset of the images
    procs.append( subprocess.Popen(["../common/run-pipe.py",
                            str(version),
                            str(start_img_id),
                            str(end_img_id),
                            in_img_dir,
                            out_img_dir,
                            str(x)
                           ]) )

    # Update the bounds for the next script
    start_img_id = start_img_id + imgs_per_thread
    end_img_id   = end_img_id   + imgs_per_thread

  proc_states = [proc.poll() for proc in procs]

  # Check every minute to see if all threads have finished
  while( all(proc_state == None for proc_state in proc_states)):
    # Previous check to see if processes have completed
    proc_states = [proc.poll() for proc in procs]
    time.sleep(5)
    
  procs[:] = []

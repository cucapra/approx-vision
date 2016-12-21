#! /usr/bin/env python

import subprocess
import sys
from os import listdir
from os.path import isfile, join


if len(sys.argv) < 4:
  print('Usage: python coco-sched-convert <input_data_vers_number> <converter_version_number> <num_threads>')
  quit()

in_version         = int(sys.argv[1])
version            = int(sys.argv[2])
total_num_threads  = int(sys.argv[3])

# The directory to convert
datasetpath   = '/work/mark/datasets/coco-2014/'

# Sections to convert
sections = ['train2014','test2014','val2014']

# Make the directory for the new version
subprocess.call(['mkdir',datasetpath+'v'+str(version)+'/'])

for section in sections:

  in_img_dir  = datasetpath+'v'+str(in_version)+'/'+section+'/'
  out_img_dir = datasetpath+'v'+str(version)+'/'+section+'/'

  # Make the directory for this section  
  subprocess.call(['mkdir',out_img_dir])

  if section == 'train2014':
    num_threads = total_num_threads / 2
  else: # test and validate
    num_threads = total_num_threads / 4

  # Get list of files in directory
  file_list = [f for f in listdir(in_img_dir) if f.endswith('.jpg') 
    and isfile(join(in_img_dir, f))]
  file_list.sort()

  # Number of images in the directory
  num_imgs = len(file_list)

  # Initialize bounds for running the pipe
  imgs_per_thread = num_imgs / num_threads
  start_img_id = 0
  end_img_id   = start_img_id + imgs_per_thread - 1

  # For every split
  for x in range(0,num_threads):
    # Provide a temporary folder for this script
    subprocess.call(['mkdir',in_img_dir+'temp'+str(x)])

    # In case the number of images is not a multiple of the number of threads
    if (num_imgs-start_img_id <= 2*imgs_per_thread):
      end_img_id = num_imgs - 1

    # Start a script to process a subset of the images
    pid = subprocess.Popen(["../common/run-pipe.py",
                            str(version),
                            str(start_img_id),
                            str(end_img_id),
                            in_img_dir,
                            out_img_dir,
                            str(x)
                           ])

    # Update the bounds for the next script
    start_img_id = start_img_id + imgs_per_thread
    end_img_id   = end_img_id   + imgs_per_thread

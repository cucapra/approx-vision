
from PIL import Image
import subprocess
from subprocess import call
import sys
import os
from os import listdir
from os.path import isfile, join
import time

# Run alternative pipelines on the strecha-mvs dataset

if len(sys.argv) < 3:
  print('Usage: \n ' +
        'python converter <input_data_vers_number> <converter_version_number> <number_of_threads>')
  quit()

in_version  = int(sys.argv[1])
version     = int(sys.argv[2])
num_threads = int(sys.argv[3])

datasetpath = '/work/mark/datasets/high-res-cifar-10/'
inputpath   = datasetpath + 'v' + str(in_version) + '/'
outputpath  = datasetpath + 'v' + str(version)+     '/'

call(['mkdir',outputpath])

call('make --directory ../common/ version='+str(version),shell=True)

classes = ['airplane','automobile','bird','cat','deer',
           'dog','frog','horse','ship','truck']
   
for class_ in classes:

  out_img_dir = outputpath + '/' + class_ + '/'
  in_img_dir  = inputpath  + '/' + class_ + '/'

  call(['mkdir',out_img_dir])

  # Get list of files in directory
  file_list = [f for f in listdir(in_img_dir) 
               if isfile(join(in_img_dir, f))]
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
    subprocess.call(['mkdir',in_img_dir+'temp'+str(x)])

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

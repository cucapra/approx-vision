
from PIL import Image
import subprocess
from subprocess import call
import sys
from os import listdir
from os.path import isfile, join
import psutil
import time

# Run alternative pipelines on the strecha-mvs dataset

if len(sys.argv) < 3:
  print('Usage: \n ' +
        'python voc-2007-converter <input_data_vers_number> <converter_version_number> <number_of_threads>')
  quit()

in_version  = int(sys.argv[1])
version     = int(sys.argv[2])
num_threads = int(sys.argv[3])

datasetpath = '/datasets/voc-2007/'
inputpath   = datasetpath + 'v' + str(in_version) + '/'
outputpath  = datasetpath + 'v' + str(version)+     '/'

call(['mkdir',outputpath])

call('make --directory ../common/ version='+str(version),shell=True)

meta_directories = ['Annotations', 'ImageSets', 'SegmentationClass', 'SegmentationObject']

for meta_directory in meta_directories:
  # Copy over the metadata
  call(['cp','-rf',
    inputpath  + meta_directory,
    outputpath + meta_directory ])
    
out_img_dir = outputpath + '/JPEGImages/'
in_img_dir  = inputpath  + '/JPEGImages/'

call(['mkdir',out_img_dir])

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

# Initialize the number of busy procs
num_busy_procs = num_threads

# Check every minute to see if all threads have finished
while(num_busy_procs != 0):
  ls = [psutil.Process(proc.pid) for proc in procs]

  gone, alive = psutil.wait_procs(ls, timeout=3)

  num_busy_procs = len(alive)

  time.sleep(5)

procs[:] = []

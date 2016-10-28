#! /usr/bin/env python

from PIL import Image
from subprocess import call
import sys
from os import listdir
from os.path import isfile, join

# Receive command line arguments
version      =     sys.argv[1]
start_img_id = int(sys.argv[2])
end_img_id   = int(sys.argv[3])
in_img_dir   =     sys.argv[4]
out_img_dir  =     sys.argv[5]
thread_id    = int(sys.argv[6])

# Get list of files in directory
file_list = [f for f in listdir(in_img_dir) 
               if isfile(join(in_img_dir, f))]
file_list.sort()

temp_dir = in_img_dir+'temp'+str(thread_id)+'/'

# For each image
for x in range(start_img_id,end_img_id+1):

  # Create the image name
  in_img_name = file_list[x]

  # Convert the jpg to png
  im = Image.open(in_img_dir+in_img_name) 
  im.save(out_img_dir+in_img_name+'_temp.png')

  # Use the reverse image pipeline
  call('../common/pipeline_V'+str(version) + '.o ' +
          out_img_dir + in_img_name + '_temp.png ' +
          temp_dir, shell=True)

  # Convert the result back to jpg, write it to converted folder
  im = Image.open(temp_dir+'output.png')
  im.save(out_img_dir+in_img_name)

  # Remove the temporary files
  call('rm '+out_img_dir + in_img_name + '_temp.png ',shell=True)

#! /usr/bin/env python

from PIL import Image
from subprocess import call
import sys

# Receive command line arguments
thread_id    =     sys.argv[1]
start_img_id = int(sys.argv[2])
end_img_id   = int(sys.argv[3])
img_dir      =     sys.argv[4]

# Location of temporary directory
temp_dir     = img_dir+'temp'+thread_id+'/'

# For each image
for x in range(start_img_id,end_img_id+1):

  # Create the image name
  in_img_name = str(x).zfill(6)+'.jpg'

  # Convert the jpg to png
  im = Image.open(img_dir+in_img_name) 
  im.save(temp_dir+'temp.png')

  # Use the reverse image pipeline
  call(["./convert",temp_dir+'temp.png',temp_dir])

  # Convert the result back to jpg, write it to converted folder
  im = Image.open(temp_dir+'output.png') 
  im.save(img_dir+'converted/'+in_img_name)



#! /usr/bin/env python

from __future__ import unicode_literals                                  
from PIL import Image
from subprocess import check_call                                        
from concurrent import futures                                           
import subprocess                                                        
import os                                                                
import io                                                                
import subprocess
import sys
from os import listdir
from os.path import isfile, join
import psutil
import time
import glob

vers_to_run = [ 3, 4, 5, 7, 8, 9,10,11,12,58,59,60,61,62,63,64]          
in_vers     = [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 6, 6, 6, 6, 6] 

num_threads = 16

# The directory to convert
datasetpath   = '/datasets/casia/'

def convert_img(img,in_version_path,out_version_path):

  # Make temp directory
  temp_dir = 'temp_'+str(os.getpid())
  subprocess.call('mkdir -p '+temp_dir,shell=True)

  # Run the given pipeline on the png
  subprocess.call('../common/pipeline_V'+str(version) + '.o ' +                     
          in_version_path + img + ' ' +                    
          temp_dir + '/', shell=True)

  # Copy to the destination directory
  subprocess.call('cp '+temp_dir+'/output.png '+
          out_version_path + img,shell=True)

  # Delete temp directory
  subprocess.call('rm -rf '+temp_dir,shell=True)


for i, version in enumerate(vers_to_run):

  in_version = in_vers[i]

  in_version_path  = datasetpath+'v'+str(in_version)
  out_version_path = datasetpath+'v'+str(version)

  # Get list of sub-directories
  subds = [ (s.rstrip("/"))[len(in_version_path):] for s in glob.glob(in_version_path+"/**")]

  # Make directories for each output class
  for subd in subds:
    subprocess.call('mkdir -p '+out_version_path+subd,shell=True) 

  # Get list of images to be converted
  imgs = [ (img)[len(in_version_path):] for img in glob.glob(in_version_path + '/**/*.png')]

  # Compile the converter 
  subprocess.call('make --directory ../common/ version='+str(version),shell=True) 

  with futures.ProcessPoolExecutor(max_workers=num_threads) as executor:
    fs = [executor.submit(
            convert_img,img,in_version_path,out_version_path)
              for img in imgs]  
    for i, f in enumerate(futures.as_completed(fs)):                   
      # Write progress to error so that it can be seen                 
      sys.stderr.write( \
        "Converted Image: {} / {} \r".format(i, len(imgs)))   


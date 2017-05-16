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

vers_to_run = [ 1]
in_vers     = [ 0]

num_threads = 4

# The directory to convert
datasetpath   = '/datasets/'


def convert_img(file_name,in_img_dir,out_img_dir):

  # Make temp directory
  temp_dir = 'temp_'+str(os.getpid())
  subprocess.call('mkdir -p '+temp_dir,shell=True)

  # Convert to png #
  im = Image.open(in_img_dir+file_name)
  im.save(temp_dir+'/'+file_name+'_temp.png')

  # Run the given pipeline on the png
  subprocess.call('../common/pipeline_V'+str(version) + '.o ' +                     
          temp_dir + '/' + file_name + '_temp.png ' +                     
          temp_dir + '/', shell=True)

  # Convert back to jpeg and save 
  im = Image.open(temp_dir+'/'+'output.png')                                 
  im.save(out_img_dir+'/'+file_name)  

  # Delete temp directory
  subprocess.call('rm -rf '+temp_dir,shell=True)


for i, version in enumerate(vers_to_run):

  in_version = in_vers[i]

  #subprocess.call('make --directory ../common/ version='+str(version),shell=True) 
  #
  # Copy all but the JPEG images
  #subprocess.call('rsync -av '+
  #                 datasetpath+'/v'+str(in_version)+'/ '+
  #                 datasetpath+'/v'+str(version)+' '+
  #                 '--exclude VOC2007/JPEGImages',
  #                 shell=True)

  in_img_dir  = datasetpath+'v'+str(in_version)+'/VOC2007/JPEGImages/'
  out_img_dir = datasetpath+'v'+str(version)+'/VOC2007/JPEGImages/'

  # Make the directory for this section  
  subprocess.call('mkdir -p '+out_img_dir,shell=True)
  
  # Get list of files in directory
  file_list = [f for f in listdir(in_img_dir) if 
                                          isfile(join(in_img_dir, f))]
  file_list.sort()

  with futures.ProcessPoolExecutor(max_workers=num_threads) as executor:
    fs = [executor.submit(convert_img,file_name,in_img_dir,out_img_dir) for file_name in file_list]  
    for i, f in enumerate(futures.as_completed(fs)):                   
      # Write progress to error so that it can be seen                 
      sys.stderr.write( \
        "Converted Image: {} / {} \r".format(i, len(file_list)))   



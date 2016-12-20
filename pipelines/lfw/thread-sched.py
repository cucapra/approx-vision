from PIL import Image
import subprocess
from subprocess import call
import sys
from os import listdir
from os.path import isfile, join

# Receive command line arguments
version      =     sys.argv[1]
start_dir_id = int(sys.argv[2])
end_dir_id   = int(sys.argv[3])
in_dir_dir   =     sys.argv[4]
out_dir_dir  =     sys.argv[5]
thread_id    = int(sys.argv[6])

# Make a list of directories
dir_list = [f for f in listdir(in_dir_dir)]
dir_list.sort()

# For each directory
for x in range(start_dir_id,end_dir_id+1):

  inputpath   = in_dir_dir  + dir_list[x] + '/'
  outputpath  = out_dir_dir + dir_list[x] + '/'

  call(['mkdir',outputpath])

  # Get list of files in directory
  file_list = [f for f in listdir(inputpath)
                 if isfile(join(inputpath, f))]
  file_list.sort()

  # Number of images in the directory
  num_imgs = len(file_list)

  # Provide a temporary folder for this script
  subprocess.call(['mkdir',inputpath+'temp'+str(thread_id)])
  
  # Process all images in this directory and wait till complete
  pid = subprocess.call("../common/run-pipe.py"+" "+
                            str(version)+" "+
                            str(0)+" "+
                            str(num_imgs-1)+" "+
                            inputpath+" "+
                            outputpath+" "+
                            str(thread_id)
                           ,shell=True)
  
  # Remove the temporary directory
  subprocess.call('rm -rf '+inputpath+'temp'+str(thread_id),shell=True)


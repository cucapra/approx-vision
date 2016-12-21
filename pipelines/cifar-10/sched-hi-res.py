
from subprocess import call
import sys
from os import listdir
from os.path import isfile, join

# Schedule multiple conversions for the high resolution cifar-10 dataset

num_threads = 16

vers_to_run = [73,74,75]
in_vers     = [ 0, 0, 0]

for index in range(0,len(vers_to_run)):
  call('python cifar-10-hi-res-converter.py '
         +str(in_vers[index])+' '
         +str(vers_to_run[index])+' '
         +str(num_threads), shell=True)

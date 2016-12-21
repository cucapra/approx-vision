
from subprocess import call
import sys
from os import listdir
from os.path import isfile, join

# Schedule conversion for the normal 32x32 cifar-10 dataset

vers_to_run = [73,74,75]
in_vers     = [ 0, 0, 0]

for index in range(0,len(vers_to_run)):
  call('python cifar-convert.py '+str(in_vers[index])+
  ' '+str(vers_to_run[index]), shell=True)

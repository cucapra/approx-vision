from subprocess import call
import sys
from os import listdir
from os.path import isfile, join

# Schedule multiple conversions for the middlebury-opt-flow dataset

#vers_to_run = [12,13,14,15,16,17]
#in_vers     = [ 0, 0, 1, 1, 1, 1]

#vers_to_run = [18,19,20,21,22,23]
#in_vers     = [ 1, 1, 1, 2, 2, 2]

vers_to_run = [24,25,26,27,36,37]
in_vers     = [ 2, 2, 2, 2, 0, 0]

for index in range(0,len(vers_to_run)):
  call('python middlebury-stereo-converter.py '+str(in_vers[index])+' '+str(vers_to_run[index]), shell=True)



from subprocess import call
import sys
from os import listdir
from os.path import isfile, join

# Schedule multiple conversions for the middlebury-opt-flow dataset

# Version numbers for each version to run
vers_to_run = [14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 36, 37]
# Version numbers paired with the input, specifically which versions are input
in_vers     = [ 1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  0,  0]


for index in range(0,len(vers_to_run)):
  call('python middlebury-opt-flow-converter.py '
          +str(in_vers[index])+' '+str(vers_to_run[index]), shell=True)



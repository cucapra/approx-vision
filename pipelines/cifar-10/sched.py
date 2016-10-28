from subprocess import call
import sys
from os import listdir
from os.path import isfile, join

# Schedule multiple conversions for cifar-10

# Version numbers for each version to run
vers_to_run = [21, 22, 23, 24, 25, 26, 27]
# Version numbers paired with the input, specifically which versions are input
in_vers     = [ 2,  2,  2,  2,  2,  2,  2]


for index in range(0,len(vers_to_run)):
  call('python cifar-convert.py '+str(in_vers[index])+' '+str(vers_to_run[index]), shell=True)


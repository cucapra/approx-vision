from subprocess import call
import sys
from os import listdir
from os.path import isfile, join

# Schedule multiple conversions for the strecha-mvs dataset

#vers_to_run = [14,15,16,17,18,19,20]
#in_vers     = [ 1, 1, 1, 1, 1, 1, 1]

vers_to_run = [21,22,23,24,25,26,27]
in_vers     = [ 2, 2, 2, 2, 2, 2, 2]

for index in range(0,len(vers_to_run)):
  call('python strecha-mvs-converter.py '+str(in_vers[index])+' '+str(vers_to_run[index]), shell=True)



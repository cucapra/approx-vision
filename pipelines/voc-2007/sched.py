
from subprocess import call
import sys
from os import listdir
from os.path import isfile, join

num_threads = 10

vers_to_run = [13,41, 6,65,67,69,71]
in_vers     = [ 0, 0, 0,13,13,13,13]

for index in range(0,len(vers_to_run)):
  call('python voc-2007-converter.py '
         +str(in_vers[index])+' '
         +str(vers_to_run[index])+' '
         +str(num_threads), shell=True)

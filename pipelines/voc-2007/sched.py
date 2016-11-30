
from subprocess import call
import sys
from os import listdir
from os.path import isfile, join

# Schedule multiple conversions for the strecha-mvs dataset

num_threads = 8

vers_to_run = [ 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,
               38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,
               65,66,67,68,69,70,71]
in_vers     = [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2,
                0, 0, 0, 0, 0, 0, 7, 7, 7, 7, 7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 6, 6, 6, 6, 6, 6, 6,
               13,13,13,13,13,13,13]


for index in range(0,len(vers_to_run)):
  call('python voc-2007-converter.py '
         +str(in_vers[index])+' '
         +str(vers_to_run[index])+' '
         +str(num_threads), shell=True)

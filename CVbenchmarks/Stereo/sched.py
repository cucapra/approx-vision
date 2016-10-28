from subprocess import call
import sys
from os import listdir
from os.path import isfile, join

# Schedule multiple optical flow error tests

# Version numbers for each version to run
#vers_to_run = [1]
vers_to_run = [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,36,37]

for index in range(0,len(vers_to_run)):
  print 'Version ' + str(vers_to_run[index])

  call('python stereo.py '+str(vers_to_run[index])+
         ' > log_'+str(vers_to_run[index])+'.txt', shell=True)



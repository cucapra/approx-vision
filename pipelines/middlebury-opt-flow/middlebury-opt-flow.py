
from subprocess import call
import sys
from os import listdir
from os.path import isfile, join

# Run alternative pipelines on the middlebury-opt-flow dataset

if len(sys.argv) < 3:
  print('Usage: python middlebury-opt-flow <input_data_vers_number> <converter_version_number>')
  quit()

in_version = int(sys.argv[1])
version    = int(sys.argv[2])

image_dirs = ['Dimetrodon','Grove2','Grove3','Hydrangea','RubberWhale','Urban2','Urban3','Venus']

datasetpath = '/datasets/middlebury-opt-flow/'
inputpath   = datasetpath + 'v' + str(in_verson) + '/'
outputpath  = datasetpath + 'v' +str(version)+'/'

call(['mkdir',outputpath])

call('make --directory ../common/ version='+str(version),shell=True)

for image_dir in image_dirs:
  call(['mkdir',outputpath+image_dir])

  print "Converting: "+image_dir

  in_img0  = inputpath  +image_dir+ '/frame10.png'
  out_img0 = outputpath +image_dir+ '/frame10.png'
  call('../common/pipeline_V'+str(version)+'.o '
         +in_img0 + ' ' + outputpath, shell=True)
  call("mv "+outputpath+'output.png '+out_img0,shell=True);

  in_img1  = inputpath  +image_dir+ '/frame11.png'
  out_img1 = outputpath +image_dir+ '/frame11.png'
  call('../common/pipeline_V'+str(version)+'.o '
         +in_img1 + ' ' + outputpath, shell=True)
  call("mv "+outputpath+'output.png '+out_img1,shell=True);





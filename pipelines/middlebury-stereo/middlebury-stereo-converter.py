
from subprocess import call
import sys
from os import listdir
from os.path import isfile, join

# Run alternative pipelines on the middlebury-stereo dataset

if len(sys.argv) < 3:
  print('Usage: python middlebury-stereo <input_data_vers_number> <converter_version_number>')
  quit()

in_version = int(sys.argv[1])
version    = int(sys.argv[2])

image_dirs = ['Aloe','Baby1','Baby2','Baby3','Bowling1','Bowling2','Cloth1','Cloth2','Cloth3','Cloth4','Flowerpots','Lampshade1','Lampshade2','Midd1','Midd2','Monopoly','Plastic','Rocks1','Rocks2','Wood1','Wood2']

datasetpath = '/datasets/middlebury-stereo/'
inputpath  = datasetpath + 'v' + str(in_version) + '/' 
outputpath = datasetpath + 'v' + str(version)    + '/' 

call(['mkdir',outputpath])

call('make --directory ../common/ version='+str(version),shell=True)

for image_dir in image_dirs:
  call(['mkdir',outputpath+image_dir])

  print "Converting: "+image_dir

  in_img0  = inputpath  +image_dir+ '/view1.png'
  out_img0 = outputpath +image_dir+ '/view1.png'
  call('../common/pipeline_V'+str(version)+'.o '
         +in_img0 + ' ' + outputpath, shell=True)
  call("mv "+outputpath+'output.png '+out_img0,shell=True);

  in_img1  = inputpath  +image_dir+ '/view5.png'
  out_img1 = outputpath +image_dir+ '/view5.png'
  call('../common/pipeline_V'+str(version)+'.o '
         +in_img1 + ' ' + outputpath, shell=True)
  call("mv "+outputpath+'output.png '+out_img1,shell=True);


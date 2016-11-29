
from PIL import Image
from subprocess import call
import sys
from os import listdir
from os.path import isfile, join

# Run alternative pipelines on the strecha-mvs dataset

if len(sys.argv) < 3:
  print('Usage: \n ' +
        'python strecha-mvs-converter <input_data_vers_number> <converter_version_number>')
  quit()

in_version = int(sys.argv[1])
version    = int(sys.argv[2])

image_dirs = ['castle-P19','castle-P30','entry-P10','fountain-P11','Herz-Jesus-P8','Herz-Jesus-P25']
#image_dirs = ['castle-P19','castle-P30']

datasetpath = '/datasets/strecha-mvs/'
inputpath   = datasetpath + 'v' + str(in_version) + '/'
outputpath  = datasetpath + 'v' + str(version)+     '/'

call(['mkdir',outputpath])

call('make --directory ../common/ version='+str(version),shell=True)

for image_dir in image_dirs:

  output_image_path = outputpath + image_dir + '/images/'
  input_image_path  = inputpath  + image_dir + '/images/'

  call(['mkdir',outputpath+image_dir])
  call(['mkdir',output_image_path])

  # Copy over the camera details
  call(['cp','-rf',
    inputpath  + image_dir + '/gt_dense_cameras',
    outputpath + image_dir + '/gt_dense_cameras'])
  call(['cp',
    input_image_path  + '/K.txt',
    output_image_path + '/K.txt'])

  # Get list of files in directory
  file_list = [f for f in listdir(input_image_path) 
                   if f.endswith('.jpg')]
  file_list.sort()

  num_imgs  = len(file_list);

  # For each image
  for x in range(0,num_imgs):
    # Create the image name
    in_img_name = file_list[x]

    # Convert the jpg to png
    im = Image.open(input_image_path+in_img_name)
    im.save(output_image_path+in_img_name+'_temp.png')

    # Use the reverse image pipeline
    call('../common/pipeline_V'+str(version) + '.o ' +
            output_image_path + in_img_name + '_temp.png ' +
            output_image_path, shell=True)

    # Convert the result back to jpg, write it to converted folder
    im = Image.open(output_image_path+'output.png')
    im.save(output_image_path+in_img_name)

    # Remove the temporary files
    call('rm '+output_image_path + in_img_name + '_temp.png ',shell=True)
    call('rm '+output_image_path+'output.png',shell=True)

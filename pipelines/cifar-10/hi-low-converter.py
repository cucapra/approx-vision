#####################################################################
# Script for converting high resolution images to CIFAR-10 binaries #
#####################################################################

import sys
import subprocess
from os import listdir
from os.path import isfile, join
import cv2
import numpy as np
import struct

# Percentage of data to be allocated to training
perc_train_data = 80

in_dataset_dir  = '/datasets/high-res-cifar-10/v0/'
out_dataset_dir = '/datasets/cifar-10/v72/'


classes = ['airplane','automobile','bird','cat','deer',
           'dog','frog','horse','ship','truck']
bin_file_names = ['data_batch_1.bin','data_batch_2.bin','data_batch_3.bin',
                  'data_batch_4.bin','data_batch_5.bin','test_batch.bin']
num_imgs = float('inf')
image_filenames = []

# Make the directory for the output
subprocess.call(['mkdir',out_dataset_dir])

# Generate list of training and testing images
for class_ in classes:
  in_img_dir = in_dataset_dir + '/' + class_
  file_list = [f for f in listdir(in_img_dir)
               if isfile(join(in_img_dir, f))]
  file_list.sort()

  if num_imgs > len(file_list):
    num_imgs = len(file_list)

  image_filenames.append(file_list)


num_train_imgs = int( num_imgs * (         perc_train_data / 100.0 ) )
num_test_imgs  = int( num_imgs * ( 1.0 - ( perc_train_data / 100.0 ) ) )

index_start = 0
index_stop  = num_train_imgs
image_index = 0
class_index = 0


# For each binary file
for i,bin_file_name in enumerate(bin_file_names):

  # Open the binary file for writing
  bin_file = open(out_dataset_dir+bin_file_name,"wb")

  # If testing, use the testing section of images
  if bin_file_name == 'test_batch.bin':
    index_start = num_train_imgs + 1
    index_stop  = num_imgs
    # Start at beginning of testing images
    image_index = index_start
  else:
    index_start = 0
    index_stop  = num_train_imgs

  for x in range(10000):
   
    input_file_path = ( in_dataset_dir + '/' +
                        classes[class_index] + '/' + 
                        image_filenames[class_index][image_index] )

    # Load the image
    in_image = cv2.imread(input_file_path)

    # Scale the image to 32x32
    size = 32, 32
    out_image = cv2.resize(in_image, dsize=size, fx=0, fy=0,  interpolation=cv2.INTER_AREA) 

    # Write the image to output file
    b,g,r = cv2.split(out_image.astype(int))
    b = np.reshape(b,1024)
    g = np.reshape(g,1024)
    r = np.reshape(r,1024)
    # Use CIFAR-10 binary format including class label
    image_data = np.concatenate((np.array([class_index]),r,g,b))
    data_byte_array = struct.pack('3073B',*image_data)
    bin_file.write(data_byte_array)

    # Increase the class index
    class_index = class_index + 1
    if class_index == 10:
      class_index = 0
      # Increase the image index
      image_index = image_index + 1
      if image_index == index_stop:
        image_index = index_start
  # Close the binary file
  bin_file.close()

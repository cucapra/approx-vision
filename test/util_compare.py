import numpy as np
import cv2

'''
  Utility functions for comparing images

  source: http://www.pyimagesearch.com/2014/09/15/python-compare-two-images/
'''

def mse(imageA, imageB):
  # the 'Mean Squared Error' between the two images is the
  # sum of the squared difference between the two images;
  # NOTE: the two images must have the same dimension
  err = np.sum((imageA.astype("float") - imageB.astype("float")) ** 2)
  err /= float(imageA.shape[0] * imageA.shape[1])
  
  # return the MSE, the lower the error, the more "similar"
  # the two images are
  return err


def compare_images(imageA_path, imageB_path):
  # compute the mean squared error and structural similarity
  # index for the images
  imageA = cv2.imread(imageA_path)
  imageB = cv2.imread(imageB_path)

  m = mse(imageA, imageB)
  return m
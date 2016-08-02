#! /usr/bin/env python

import matplotlib.pyplot as plt
import numpy as np

def tonemap1(x):
  if   x < 64:
    y = x * 2
  elif x < 128:
    y = x   + 64
  else:
    y = x/2 + 128
  return y

def tonemap2(x):
  if   x < 32:
    y = x * 4
  elif x < 128:
    y = x   + 96
  else:
    y = x/4 + 192
  return y

def tonemap3(x):
  if   x < 16:
    y = x * 8
  elif x < 32:
    y = x * 4 + 64
  elif x < 64:
    y = x     + 160
  elif x < 128:
    y = x / 4 + 208
  else:
    y = x / 8 + 224
  return y

camf_r = []
camf_g = []
camf_b = []
yplot1 = []
yplot2 = []
yplot3 = []

firstline = True
for line in open("../../ReversiblePipeline/camera_models/NikonD7000/jpg2raw_respFcns.txt"):
  listvals = line.split(" ")
  if firstline == False:
    camf_r.append(int(float(listvals[0])*255.0))
    camf_g.append(int(float(listvals[1])*255.0))
    camf_b.append(int(float(listvals[2])*255.0))
  firstline = False

for x in range(0,256):
  yplot1.append(tonemap1(x))
  yplot2.append(tonemap2(x))
  yplot3.append(tonemap3(x))


camf_r = np.array(camf_r)
camf_g = np.array(camf_g)
camf_b = np.array(camf_b)
yplot1 = np.array(yplot1)
yplot2 = np.array(yplot2)
yplot3 = np.array(yplot3)

print(camf_r.shape)
print(yplot1.shape)

plt.plot(range(0,256),yplot1,'k',range(0,256),yplot2,'k',range(0,256),yplot3,'k',
  camf_r,range(0,256),'r',camf_g,range(0,256),'g',camf_b,range(0,256),'b')

plt.show()



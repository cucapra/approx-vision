#! /usr/bin/env python

import matplotlib.pyplot as plt
import numpy as np
from math import log

def exp_tonemap(x,gamma):
  A = 255.0/(255.0**gamma)
  return A*(x**gamma)

def log_tonemap(x,in_scale):
  A = 255.0/log((255+1)*in_scale,2)
  return A*log((x+1)*in_scale,2)

camf_r = []
camf_g = []
camf_b = []
yplot1 = []
yplot2 = []
yplot3 = []
yplot4 = []
yplot5 = []
yplot6 = []
yplot7 = []
yplot8 = []


firstline = True
for line in open("../../ReversiblePipeline/camera_models/NikonD7000/jpg2raw_respFcns.txt"):
  listvals = line.split(" ")
  if firstline == False:
    camf_r.append(int(float(listvals[0])*255.0))
    camf_g.append(int(float(listvals[1])*255.0))
    camf_b.append(int(float(listvals[2])*255.0))
  firstline = False

for x in range(0,256):
  yplot1.append(exp_tonemap(x,1.0/2.0 ))
  yplot2.append(exp_tonemap(x,1.0/4.0 ))
  yplot3.append(exp_tonemap(x,1.0/8.0 ))
  yplot4.append(exp_tonemap(x,1.0/16.0))
  yplot5.append(log_tonemap(x,0.3 ))
  yplot6.append(log_tonemap(x,0.3 ))
  yplot7.append(log_tonemap(x,5.0 ))
  yplot8.append(log_tonemap(x,500.0))

camf_r = np.array(camf_r)
camf_g = np.array(camf_g)
camf_b = np.array(camf_b)
yplot1 = np.array(yplot1)
yplot2 = np.array(yplot2)
yplot3 = np.array(yplot3)
yplot4 = np.array(yplot4)
yplot5 = np.array(yplot5)
yplot6 = np.array(yplot6)
yplot7 = np.array(yplot7)
yplot8 = np.array(yplot8)


print(camf_r.shape)
print(yplot1.shape)

plt.plot(range(0,256),yplot1,'k',range(0,256),yplot2,'k',
  range(0,256),yplot3,'k',range(0,256),yplot4,'k',
  range(0,256),yplot5,'m',range(0,256),yplot6,'m',
  range(0,256),yplot7,'m',range(0,256),yplot8,'m',
  camf_r,range(0,256),'r',camf_g,range(0,256),'g',camf_b,range(0,256),'b')

plt.show()



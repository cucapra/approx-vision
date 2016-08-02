#! /usr/bin/env python

import matplotlib.pyplot as plt

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


yplot1 = []
yplot2 = []
yplot3 = []

for x in range(0,255):
  yplot1.append(tonemap1(x))
  yplot2.append(tonemap2(x))
  yplot3.append(tonemap3(x))


plt.plot(range(0,255),yplot1,'r',range(0,255),yplot2,'b',range(0,255),yplot3,'g')

plt.show()



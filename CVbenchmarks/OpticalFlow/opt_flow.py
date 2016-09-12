import sys
import os
import numpy as np
import cv2
import ipdb
import matplotlib.pyplot as plt

#############  Utilities for Optical Flow #################################
# These were taken from https://github.com/jswulff/pcaflow 
###########################################################################
TAG_FLOAT = 202021.25
TAG_CHAR = 'PIEH'

def flow_read(filename):
    """ Read optical flow from file, return (U,V) tuple. 
    
    Original code by Deqing Sun, adapted from Daniel Scharstein.
    """
    f = open(filename,'rb')
    check = np.fromfile(f,dtype=np.float32,count=1)[0]
    assert check == TAG_FLOAT, ' flow_read:: Wrong tag in flow file (should be: {0}, is: {1}). Big-endian machine? '.format(TAG_FLOAT,check)
    width = np.fromfile(f,dtype=np.int32,count=1)[0]
    height = np.fromfile(f,dtype=np.int32,count=1)[0]
    size = width*height
    assert width > 0 and height > 0 and size > 1 and size < 100000000, ' flow_read:: Wrong input size (width = {0}, height = {1}).'.format(width,height)
    tmp = np.fromfile(f,dtype=np.float32,count=-1).reshape((height,width*2))
    u = tmp[:,np.arange(width)*2]
    v = tmp[:,np.arange(width)*2 + 1]
    return u,v

def viz_flow(u,v,logscale=True,scaledown=6,output=False):
    """
    topleft is zero, u is horiz, v is vertical
    red is 3 o'clock, yellow is 6, light blue is 9, blue/purple is 12
    """
    colorwheel = makecolorwheel()
    ncols = colorwheel.shape[0]

    radius = np.sqrt(u**2 + v**2)
    if output:
        print("Maximum flow magnitude: %04f" % np.max(radius))
    if logscale:
        radius = np.log(radius + 1)
        if output:
            print("Maximum flow magnitude (after log): %0.4f" % np.max(radius))
    radius = radius / scaledown    
    if output:
        print("Maximum flow magnitude (after scaledown): %0.4f" % np.max(radius))
    rot = np.arctan2(-v, -u) / np.pi

    fk = (rot+1)/2 * (ncols-1)  # -1~1 maped to 0~ncols
    k0 = fk.astype(np.uint8)       # 0, 1, 2, ..., ncols

    k1 = k0+1
    k1[k1 == ncols] = 0

    f = fk - k0

    ncolors = colorwheel.shape[1]
    img = np.zeros(u.shape+(ncolors,))
    for i in range(ncolors):
        tmp = colorwheel[:,i]
        col0 = tmp[k0]
        col1 = tmp[k1]
        col = (1-f)*col0 + f*col1
       
        idx = radius <= 1
        # increase saturation with radius
        col[idx] = 1 - radius[idx]*(1-col[idx])
        # out of range    
        col[~idx] *= 0.75
        img[:,:,i] = np.floor(255*col).astype(np.uint8)
    
    return img.astype(np.uint8)
    
   
    
def makecolorwheel():
    # Create a colorwheel for visualization
    RY = 15
    YG = 6
    GC = 4
    CB = 11
    BM = 13
    MR = 6
    
    ncols = RY + YG + GC + CB + BM + MR
    
    colorwheel = np.zeros((ncols,3))
    
    col = 0
    # RY
    colorwheel[0:RY,0] = 1
    colorwheel[0:RY,1] = np.arange(0,1,1./RY)
    col += RY
    
    # YG
    colorwheel[col:col+YG,0] = np.arange(1,0,-1./YG)
    colorwheel[col:col+YG,1] = 1
    col += YG
    
    # GC
    colorwheel[col:col+GC,1] = 1
    colorwheel[col:col+GC,2] = np.arange(0,1,1./GC)
    col += GC
    
    # CB
    colorwheel[col:col+CB,1] = np.arange(1,0,-1./CB)
    colorwheel[col:col+CB,2] = 1
    col += CB
    
    # BM
    colorwheel[col:col+BM,2] = 1
    colorwheel[col:col+BM,0] = np.arange(0,1,1./BM)
    col += BM
    
    # MR
    colorwheel[col:col+MR,2] = np.arange(1,0,-1./MR)
    colorwheel[col:col+MR,0] = 1

    return colorwheel    

###################### Main Script ###############################


files = ['Dimetrodon','Grove2','Grove3','Hydrangea','RubberWhale','Urban2','Urban3','Venus']

for f in files:

    im1 = cv2.imread('data/'+f+'/frame10.png',0)
    im2 = cv2.imread('data/'+f+'/frame11.png',0)

    gt_u, gt_v = flow_read('groundtruth/'+f+'/flow10.flo')
    gt = viz_flow(gt_u,gt_v)

    plt.imshow(gt)
    plt.show()


    flow = cv2.calcOpticalFlowFarneback(im1,im2, 0.5, 3, 15, 3, 5, 1.2, 0)
    calc_flow = viz_flow(flow[:,:,0],flow[:,:,1])
    plt.imshow(calc_flow)
    plt.show()

    error = error = ((gt-calc_flow)**2).mean()
    print error

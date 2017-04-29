
#####################################################################
# Image Pipeline Scripts
#####################################################################

#####################################################################
# General Description
#####################################################################

This directory contains both ImgPipe.cpp and RawPreproc.cpp.
ImgPipe.cpp can be used as a non-configurable, non-reversible, 
image processing pipeline. RawPreproc is used to create images
usable as raw input to the Halide and Matlab imaging pipelines. 

#####################################################################
# How to build
#####################################################################

  1) Install LibRaw and OpenCV
    LibRaw docs: http://www.libraw.org/docs/API-CXX-eng.html
    OpenCV docs: http://docs.opencv.org/3.1.0/
  2) Change to this directory (ReversiblePipeline/src/scripts)
  3) Compile with the provided Makefile
    > make
  4) Determine the bitdepth of your raw input file
  5) Run either script, input format is the same
    > ./RawPreproc path/to/image <raw file bitdepth>
    example:
    > ./RawPreproc ../../imgs/NikonD7000FL/DSC_0916.NEF 14
  6) Observe the results in the same directory that your input
     file is in.

#####################################################################
# Additional notes
#####################################################################

Both the RawPreproc and ImgPipe scripts in this directory perform
demosaicing with OpenCV. To demosaic correctly, the correct bayer 
pattern must be used. The current implementation doesn't provide
easy configurability for changing the bayer filter version, but 
once you know what pattern your raw files use, you can change the 
pattern in the code according to this guide:

http://docs.opencv.org/2.4/modules/imgproc/doc/miscellaneous_transformations.html

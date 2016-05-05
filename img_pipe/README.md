

This directory contains code to simulate the image processing
pipeline. This includes camera sensing through the ISP to 
JPEG compression. This is work-in-progress. 

The tunable pipeline is built by connecting various processing
stages. Each stage will eventually have a forward function which
will perform the relevant computation on the input image, as 
well as produce statistics needed to compute energy and delay.
Stage by stage backward functions may be necessary in future
so that these steps can be reversed.


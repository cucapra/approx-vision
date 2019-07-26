# The Approximate Vision Project

This is the public release of code developed for the paper ["Reconfiguring the Imaging Pipeline for Computer Vision"](https://capra.cs.cornell.edu/research/visionmode/) by Mark Buckler, Suren Jayasuriya, and Adrian Sampson. It contains the Configurable & Reversible Imaging Pipeline (CRIP) described in the paper, documentation on how to run and edit the CRIP for your own use, and both Dockerfiles and instructions for how to run our supported computer vision benchmarks.

# License and citation

All code in this git repository is released under the MIT license. If you use this code in your research, please our ICCV 2017 paper:

```
@inproceedings{buckler-iccv2017,
    author = {Mark Buckler and Suren Jayasuriya and Adrian Sampson},
    title = {Reconfiguring the Imaging Pipeline for Computer Vision},
    booktitle = {The IEEE International Conference on Computer Vision (ICCV)},
    year = {2017},
}
```

# Compiling, running, and general usage

All available documentation for this code can be found in [this GitHub repo's Wiki](https://github.com/cucapra/approx-vision/wiki). Those of you who just want to run a simple example or are just getting started will find our [FAQ](https://github.com/cucapra/approx-vision/wiki/Getting-Started-FAQ) particularly helpful.

# Learning and using other camera models

Our pipeline is an augmented version of this [reversible imaging
pipeline](https://github.com/mbuckler/ReversiblePipeline). The page that used to
host additional models as well as the code needed to learn new models is
[here](https://www.comp.nus.edu.sg/~brown/radiometric_calibration/) but
unfortunately the site appears to be down. This is possibly because the PI on
the project [Michael S. Brown](https://www.eecs.yorku.ca/~mbrown/) has moved
universities. If you contact them you may gain access to the original code, but
we don't have access.

# Contributors

 * Mark Buckler (mab598@cornell.edu)
 * Suren Jayasuriya (sjayasur@andrew.cmu.edu)
 * Adrian Sampson (asampson@cs.cornell.edu)
 * Taehoon Lee (tl353@cornell.edu)

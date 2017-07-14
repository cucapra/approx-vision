from PIL import Image

import ast
import argparse
import ConfigParser
import os
import subprocess, sys, shutil
import pdb


CONFIG_FILE = "/approx-vision/pipelines/CLI/config/config.ini"
IMAGE_TEMP_IN = "/approx-vision/pipelines/CLI/temp/temp_in.png"
IMAGE_TEMP_OUT = "/approx-vision/pipelines/CLI/temp/output/output.png"


'''
  Module for running input image through image processing pipeline.


  Options:
    path to input image file
    path to output image file
    path to camera model
    wb index
    num of control points
    pipeline version #
    pipeline version stages
    path to pipeline source cpp code
    True to build pipeline source cpp code, False otherwise
'''
class Pipeline(object):

  def __init__(self, in_file_path, out_file_path, cam_model_path, \
                      wb_index, num_ctrl_pts, version, stages,
                      pipeline_path, build ):
    
    self.in_file_path = in_file_path
    self.out_file_path = out_file_path
    self.cam_model_path = cam_model_path
    self.wb_index = wb_index
    self.num_ctrl_pts = num_ctrl_pts
    self.version = version
    self.stages = stages
    self.pipeline_path = pipeline_path
    self.pipeline_folder = os.path.dirname(pipeline_path)
    self.pipeline_obj_path = pipeline_path[:-3] + "o"
    self.build = build

  def run(self):
    print "\nStarted running pipeline version # " + str(self.version)

    try:

      # build pipeline source files
      if self.build:
        command = ["make", "--directory", self.pipeline_folder]
        subprocess.call(" ".join(command), shell=True)

      # create temp image file
      img_filename = os.path.basename(self.in_file_path)
      img = Image.open(self.in_file_path)
      img.save(IMAGE_TEMP_IN)

      # change working dir
      os.chdir(self.pipeline_folder)

      # call pipeline
      command = [self.pipeline_obj_path, IMAGE_TEMP_IN, self.out_file_path]
      command.extend([self.cam_model_path, self.wb_index, self.num_ctrl_pts])
      command.append(self.stages)
      print " ".join(command)
      subprocess.call(" ".join(command), shell=True)

      # move output to output folder
      # out_file_path = os.path.join(self.out_dir_path, img_filename)
      # temp_out_file_path = IMAGE_TEMP_OUT + "output.png"
      # img = Image.open(temp_out_file_path)
      # img.save(out_file_path)

      # command = ["rm", "-rf", IMAGE_TEMP_IN, temp_out_file_path ]
      # subprocess.call(" ".join(command), shell=True)

      print "Finished running pipeline version # " + str(self.version)

    except Exception as e:
      print "Failed to run pipeline: "
      print e


###############################################################################

'''
  Command line tool for converting image data via defined image pipeline

  argv:
    path to input image file
    path to output image directory
    path to camera model
    wb index
    num of control points
    version #
'''
if __name__ == "__main__":

  # config parsing
  config = ConfigParser.ConfigParser()
  config.read(CONFIG_FILE)

  cam_model_path  = config.get("default", "cam_model_path") 
  wb_index        = config.get("default", "wb_index") 
  num_ctrl_pts    = config.get("default", "num_ctrl_pts")
  pipeline_path   = config.get("default", "pipeline_path")

  # args
  parser = argparse.ArgumentParser(description="Runs Pipeline")
  parser.add_argument("--build",
    default="True",
    help="'True' to compile pipeline source files, otherwise set to 'False'")
  parser.add_argument("--infile",
    default=IMAGE_TEMP_IN,
    help="Filepath to input image file")
  parser.add_argument("--outfile",
    default=IMAGE_TEMP_OUT,
    help="Filepath to output image file")
  parser.add_argument("--version",
    default=1,
    help="Pipeline version to run")
  parser.add_argument("--campath",
    default=cam_model_path,
    help="Path to camera model")
  parser.add_argument("--wb",
    default=wb_index,
    help="White balance index")
  parser.add_argument("--ctrl",
    default=num_ctrl_pts,
    help="Number of control points")

  args = parser.parse_args()

  version_stages = config.get("version", "V"+str(args.version))
  version_stages = " ".join(version_stages.split("/"))

  pipeline = Pipeline(args.infile, args.outfile, args.campath, args.wb, args.ctrl, \
                      args.version, version_stages, pipeline_path, \
                      ast.literal_eval(args.build))

  pipeline.run()
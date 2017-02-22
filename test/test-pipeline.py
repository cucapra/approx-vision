from PIL import Image
import ast
import ConfigParser
import os
import subprocess, sys, shutil
import pdb


class TestPipeline(object):

  def __init__(self, datasets, datasets_bin, versions, \
                pipeline_prefix, \
                ref_dir, in_dir, out_dir):
    self.datasets = datasets
    self.datasets_bin = datasets_bin
    self.versions = versions

    self.pipeline_prefix = pipeline_prefix

    self.ref_dir = ref_dir
    self.in_dir = in_dir
    self.out_dir = out_dir


  ''' Run all images for specified dataset and pipeline version.
  '''
  def run_pipeline(self, dataset, version):

    # check valid dataset & version
    if not(dataset in self.datasets) or not(version in self.versions):
      print "invalid dataset(%s) or version(%d)" % (dataset, version)
      return -1

    # clear out out dataset folder
    out_dataset_version_dir = self.get_dir_path(self.out_dir, dataset, version)
    if os.path.exists(out_dataset_version_dir):
      shutil.rmtree(out_dataset_version_dir)
    os.makedirs(out_dataset_version_dir)

    # newly compile pipeline version
    pipeline_filename = self.get_pipeline_filename(dataset, version)
    pipeline_folder = os.path.dirname(pipeline_filename)
    command = ["make", "--directory", pipeline_folder, "version="+str(version)]
    subprocess.call(' '.join(command), shell=True)

    # get original file
    in_dataset_dir = os.path.join(self.in_dir, dataset)
    filename = self.get_first_filename(in_dataset_dir)
    if not filename:
      return -1
    in_filepath = os.path.join(in_dataset_dir, filename)
    out_filepath = os.path.join(out_dataset_version_dir, filename)

    # convert to png
    img_temp_in = in_filepath + "_temp.png"
    img_temp_out = os.path.join(out_dataset_version_dir, "output.png")

    img = Image.open(in_filepath)
    img.save(img_temp_in)

    # call pipeline
    command = [pipeline_filename, img_temp_in, out_dataset_version_dir + "/"]
    print ' '.join(command)
    subprocess.call(' '.join(command), \
                    shell=True)

    # convert back
    img = Image.open(img_temp_out)
    img.save(out_filepath)

    # remove temp files
    subprocess.call(["rm", "-rf", img_temp_in, img_temp_out], shell=True)

    return 0

  ''' compare 
  '''
  def compare_pipeline(self, dataset, version):

    # get test target file
    out_folder = self.get_dir_path(self.out_dir, dataset, version)
    filename = self.get_first_filename(out_folder)
    if not filename:
      return -1
    out_filepath = os.path.join(out_folder, filename) 

    # check corresponding ref file exists
    ref_folder = self.get_dir_path(self.ref_dir, dataset, version)
    ref_filepath = os.path.join(ref_folder, filename)
    if not os.path.exists(ref_filepath):
      print "no ref test file exists: %s" % ref_filepath
      return -1

    # compare the two files

    return False


  ''' returns first filename in folder or None if no such file
  '''
  def get_first_filename(self, folder_path):
    files = [f for f in os.listdir(folder_path) \
                if os.path.isfile(os.path.join(folder_path, f))]
    if len(files) == 0:
      print "no target test file exists: %s" % folder_path
      return None

    files.sort()
    
    return files[0]

  ''' returns joined file path
  '''
  def get_dir_path(self, root, dataset, version):
    return os.path.join(root, dataset, "V" + str(version))

  ''' returns name of corresponding pipeline file name
  '''
  def get_pipeline_filename(self, dataset, version):
    if dataset in self.datasets_bin:
      print "not yet supported"
      return ""
    else:
      return self.pipeline_prefix + str(version) + ".o"


###############################################################################

''' Read in a list from config parser
'''
def config_parser_list(parser, section, item):
  return ast.literal_eval(parser.get(section, item))

if __name__ == "__main__":
  config = ConfigParser.ConfigParser()
  config.read("config.ini")

  datasets = config_parser_list(config, "main", "datasets")
  datasets_bin = config_parser_list(config, "main", "datasets_bin")
  versions = config_parser_list(config, "main", "versions")

  pipeline_prefix = config.get("main", "pipeline_prefix")

  ref_dir = config.get("reference", "dataset_path")
  in_dir = config.get("main", "dataset_path")
  out_dir = config.get("test", "dataset_path")

  tp = TestPipeline(datasets, datasets_bin, versions, pipeline_prefix, \
                    ref_dir, in_dir, out_dir)

  tp.run_pipeline("coco-2014", 5)

  pdb.set_trace()

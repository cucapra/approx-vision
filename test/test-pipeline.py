from PIL import Image
import ast
import ConfigParser
import os
import subprocess, sys, shutil
import util_compare

'''

'''
class TestPipeline(object):

  def __init__(self, datasets, datasets_bin, versions, pipeline_prefix, \
                ref_dir, in_dir, out_dir):
    self.datasets = datasets
    self.datasets_bin = datasets_bin
    self.versions = versions

    self.pipeline_prefix = pipeline_prefix

    self.ref_dir = ref_dir
    self.in_dir = in_dir
    self.out_dir = out_dir


  ''' Produce new image by running pipeline specified by dataset & version.
      Args
        dataset(String): name of dataset for pipeline to run
        version(Integer): version number for pipeline to run
        rebuild(Boolean): rebuild pipeline cpp if set to True

      * uses images in in_dir and produces new images in out_dir 
  '''
  def run_pipeline(self, dataset, version, rebuild):

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
    if rebuild:
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

    # change working dir
    os.chdir(pipeline_folder)

    # call pipeline
    command = [pipeline_filename, img_temp_in, out_dataset_version_dir + "/"]
    subprocess.call(' '.join(command), shell=True)

    # convert back
    img = Image.open(img_temp_out)
    img.save(out_filepath)

    # remove temp files
    command = ["rm", "-rf", img_temp_in, img_temp_out]
    subprocess.call(' '.join(command), shell=True)

    return 0

  ''' Compares images to the reference images. 
      Returns True if images are the same, False otherwise

      Args
        dataset(String): name of the dataset for pipeline
        version(Integer): version number for pipeline

      Returns
        True if images are the same. False otherwise 
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

    # compare the two image files
    mse_diff = util_compare.compare_images(out_filepath, ref_filepath)

    print "file: %s, mse: %f" % (out_filepath, mse_diff)

    if mse_diff > 0.0:
      return False

    return True


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

  if len(sys.argv) != 2 or (sys.argv[1] != "ref" and sys.argv[1] != "test"):
    print "\nUsage:\n"
    print "\ttest-pipeline ref : generates new reference files\n"
    print "\ttest-pipeline test : generates new test files and compare them with reference files\n"
    sys.exit()

  ref_mode = False
  if sys.argv[1] == "ref":
    sys.stdout.write("Are you sure you want to generate reference images? Type 'yes' to confirm.")
    user_input = raw_input().lower()
    if user_input != "yes":
      sys.exit()

    ref_mode = True

  # config
  config = ConfigParser.ConfigParser()
  config.read("config.ini")

  datasets    = config_parser_list(config, "main", "datasets")
  datasets_bin = config_parser_list(config, "main", "datasets_bin")
  versions = config_parser_list(config, "main", "versions")
  pipeline_prefix = config.get("main", "pipeline_prefix")
  in_dir = config.get("main", "dataset_path")
  ref_dir = config.get("reference", "dataset_path")
  out_dir = config.get("test", "dataset_path")

  # ref mode: generate reference files based on current pipeline implementations
  if ref_mode:
    out_dir = ref_dir

  # test mode: generate test files based on current pipeline implementations
  #            then compare these files with corresponding reference files

  tp = TestPipeline(datasets, datasets_bin, versions, pipeline_prefix, \
                    ref_dir, in_dir, out_dir)

  # run for each dataset and version combination
  test_passed = True
  for version_num in versions:
    rebuild = True

    for dataset_name in datasets:
      tp.run_pipeline(dataset_name, version_num, rebuild)
      rebuild = False

      if not ref_mode: # also compare files with ref files
        test_passed = test_passed and tp.compare_pipeline(dataset_name, version_num)

  print "Test Passed: ", test_passed


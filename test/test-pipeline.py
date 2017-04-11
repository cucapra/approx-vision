from PIL import Image
import ast
import ConfigParser
import os
import subprocess, sys, shutil
import util_compare

'''
  Module for testing whether new pipeline files produce the same output
  as original pipeline files.
'''
class TestPipeline(object):

  def __init__(self, datasets, datasets_bin, versions, pipeline_prefix, \
                ref_dir, in_dir, out_dir, \
                cam_model_path, wb_index, num_ctrl_pts ):
    self.datasets         = datasets
    self.datasets_bin     = datasets_bin
    self.versions         = versions

    self.pipeline_prefix  = pipeline_prefix

    self.ref_dir          = ref_dir
    self.in_dir           = in_dir
    self.out_dir          = out_dir

    self.cam_model_path   = cam_model_path
    self.wb_index         = wb_index
    self.num_ctrl_pts     = num_ctrl_pts

  ''' Produce new image by running pipeline specified by dataset & version.
      Args
        dataset(String): name of dataset for pipeline to run
        version(Integer): version number for pipeline to run
        rebuild(Boolean): rebuild pipeline cpp if set to True

      * uses images in in_dir and produces new images in out_dir 
  '''
  def run_pipeline(self, dataset, version, rebuild, ref_mode):

    # check valid dataset & version
    if not(dataset in self.datasets) or not(version in self.versions):
      print "invalid dataset(%s) or version(%d)" % (dataset, version)
      return -1

    # clear out output-dataset folder
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

    # get original image file
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

    # change working dir(needed for relative path in cpp files)
    os.chdir(pipeline_folder)

    # call pipeline
    command = [pipeline_filename, img_temp_in, out_dataset_version_dir + "/"]
    if (not ref_mode):
      command.extend([cam_model_path, wb_index, num_ctrl_pts])
              
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

    print "mse: %f, file: %s" % (mse_diff, out_filepath)

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
      print "binary image files: not yet supported"
      return ""
    else:
      return self.pipeline_prefix + str(version) + ".o"


###############################################################################

''' Read in a list from config parser
'''
def config_parser_list(parser, section, item):
  return ast.literal_eval(parser.get(section, item))


'''
  Runs in two modes:
    1. ref  : generates new reference files based on original pipeline files
    2. test : generates files based on new pipeline files, compares files with
              reference files
    3. ref-test: run ref mode and then test mode

'''
if __name__ == "__main__":
  modes = { # key: mode, value: (creating reference file required, modes to run)
    "ref": (True, [True]), 
    "test": (False, [False]),
    "ref-test": (True, [True, False])
  }

  if len(sys.argv) != 2 or not(sys.argv[1] in modes):
    print "\nArguments:\n"
    print "\tref\t: generates new reference files\n"
    print "\ttest\t: generates new test files and compare them with reference files\n"
    print "\tref-test\t: run ref mode and then test mode\n"
    sys.exit()
  
  global_ref_mode, modes_to_run = modes[sys.argv[1]]

  if global_ref_mode:
    sys.stdout.write("\nAre you sure you want to generate reference images? \n")
    sys.stdout.write("This action may over-write existing reference images. \n")
    sys.stdout.write("Type 'yes' to confirm.\n")
    sys.stdout.write("> ")
    user_input = raw_input().lower()
    if user_input != "yes":
      sys.exit()

  # config parsing
  config = ConfigParser.ConfigParser()
  config.read("config.ini")

  datasets        = config_parser_list(config, "main", "datasets")
  datasets_bin    = config_parser_list(config, "main", "datasets_bin")
  versions        = config_parser_list(config, "main", "versions")
  pipeline_prefix = config.get("main", "pipeline_prefix")
  in_dir          = config.get("main", "dataset_path")
  ref_dir         = config.get("reference", "dataset_path")
  out_dir         = config.get("test", "dataset_path")

  cam_model_path  = config.get("main", "cam_model_path")
  wb_index        = config.get("main", "wb_index")
  num_ctrl_pts    = config.get("main", "num_ctrl_pts")

  '''
  test mode: generate test files based on original pipeline implementations
             then compare these files with corresponding reference files

  ref mode: generate reference files based on original pipeline implementations

  ref-test mode: run ref mode and then test mode
  '''
  global_test_pass = True
  for current_mode_is_ref in modes_to_run:

    if current_mode_is_ref: # is ref mode
      out_dir         = ref_dir
      pipeline_prefix = config.get("main", "pipeline_prefix_ref")
    else:
      out_dir         = config.get("test", "dataset_path")
      pipeline_prefix = config.get("main", "pipeline_prefix")


    tp = TestPipeline(datasets, datasets_bin, versions, pipeline_prefix, \
                      ref_dir, in_dir, out_dir, \
                      cam_model_path, wb_index, num_ctrl_pts)

    # run pipeline for each dataset and version combination
    local_test_pass = True
    for version_num in versions:
      rebuild = True

      for dataset_name in datasets:
        tp.run_pipeline(dataset_name, version_num, rebuild, current_mode_is_ref)
        rebuild = False

        if not current_mode_is_ref: # also compare files with ref files
          local_test_pass = tp.compare_pipeline(dataset_name, version_num) and local_test_pass

      if not current_mode_is_ref: 
        print("Version" + str(version_num) + " Test Pass: " + str(local_test_pass) + "\n")
        global_test_pass = global_test_pass and local_test_pass

  print("\nGlobal Test Pass: " + str(global_test_pass))



import subprocess
import csv
import time

RAISE_FILE = "/approx-vision/RAW_RAISE/data/RAISE_all.csv"
INPUT_DIR = "/approx-vision/RAW_RAISE/data/dataset/"

SCIRPT_DIR = "/approx-vision/RAW_RAISE/src/conversion/"
PREPROCESS_CONVERT_SCRIPT = "/approx-vision/RAW_RAISE/src/conversion/RawToPng"
RESIZE_CONVERT_SCRIPT = "/approx-vision/RAW_RAISE/src/conversion/Resize"
FULL_CONVERT_SCRIPT = "/approx-vision/pipelines/CLI/pipeline.py"

PNG_OUTPUT_DIR = "/approx-vision/RAW_RAISE/data/dataset_preprocess/"
RAW_OUTPUT_DIR = "/approx-vision/RAW_RAISE/data/dataset_raw/"
FULL_OUTPUT_DIR = "/approx-vision/RAW_RAISE/data/dataset_full/"

WIDTH = "299"
HEIGHT = "299"

RESIZE_FACTOR = "8"

def execute(command):
  subprocess.call(" ".join(command), shell=True)

def preprocess_image(input_file, output_file_raw, output_file_full, \
                                            resize_factor, num_bits):
  command = [
              PREPROCESS_CONVERT_SCRIPT, 
              input_file, 
              output_file_raw, 
              output_file_full, 
              resize_factor, 
              num_bits
            ]
  execute(command)

def resize(input_file, output_file, width, height):
  command = [RESIZE_CONVERT_SCRIPT, input_file, output_file, width, height]
  execute(command)

if __name__ == "__main__":

  # build source
  execute(["make", "--directory", SCIRPT_DIR])

  images = [("r000da54ft", "12"), ("r001d260dt", "14"), ("r5f061417t", "14")]
  # with open(RAISE_FILE, "r") as f:
  #   reader = csv.DictReader(f)
  #   for row in reader:
  #     name = row["File"]
  #     bit = str(parse_int(row["Image Quality"]))
  #     images.append((name, bit))

  
  for i, (name, bit) in enumerate(images):
    start = time.time()

    # convert to raw and full
    input_file = INPUT_DIR + name + ".NEF"
    output_file_raw = RAW_OUTPUT_DIR + name + ".png"
    output_file_full = FULL_OUTPUT_DIR + name + ".png"
    preprocess_image(input_file, output_file_raw, output_file_full, RESIZE_FACTOR, bit)
    
    # # descale
    # input_file = PNG_OUTPUT_DIR + name + ".png"
    # output_file = RAW_OUTPUT_DIR + name + ".png"
    # command = [
    #             "python", FULL_CONVERT_SCRIPT, 
    #             "--infile", input_file,
    #             "--outfile", output_file,
    #             "--build", "False",
    #             "--version", "99"
    #           ]
    # execute(command)

    # resize png
    input_file = RAW_OUTPUT_DIR + name + ".png"
    output_file = RAW_OUTPUT_DIR + name + "_resized" + ".png"
    resize(input_file, output_file, WIDTH, HEIGHT)

    # # convert png to full pipeline png
    # input_file = PNG_OUTPUT_DIR + name + ".png"
    # output_file = FULL_OUTPUT_DIR + name + ".png"
    # command = [
    #             "python", FULL_CONVERT_SCRIPT, 
    #             "--infile", input_file,
    #             "--outfile", output_file,
    #             "--build", "False",
    #             "--version", "100"
    #           ]
    # execute(command)

    # resize full pipeline png
    input_file = FULL_OUTPUT_DIR + name + ".png"
    output_file = FULL_OUTPUT_DIR + name + "_resized" + ".png"
    resize(input_file, output_file, WIDTH, HEIGHT)

    end = time.time()

    print "preprocessed image #" + str(i) + "\t," + name + "\t, took " + str(end - start) 




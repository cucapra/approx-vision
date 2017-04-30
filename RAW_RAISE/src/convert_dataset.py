import subprocess
import csv

RAISE_FILE = "/approx-vision/RAW_RAISE/data/RAISE_all.csv"
INPUT_DIR = "/approx-vision/RAW_RAISE/data/dataset/"
OUTPUT_DIR = "/approx-vision/RAW_RAISE/data/dataset_converted/"
CONVERT_SCRIPT = "/approx-vision/RAW_RAISE/src/conversion/RawToPng"
CONVERT_DIR = "/approx-vision/RAW_RAISE/src/conversion/"
CONVERT_SUFFIX = ".3C.png"
CONVERT_WIDTH = "299"
CONVERT_HEIGHT = "299"

def execute(command):
  subprocess.call(" ".join(command), shell=True)

def convert_image(input_file, num_bits, out_width, out_height):
  command = [CONVERT_SCRIPT, input_file, num_bits, out_width, out_height]
  execute(command)

def move_image(input_file, output_dir):
  command = ["mv", input_file, output_dir]
  execute(command)

if __name__ == "__main__":

  # build source
  command = ["make", "--directory", CONVERT_DIR]
  execute(command)

  images = [("r000da54ft", "12"), ("r001d260dt", "14")]
  # with open(RAISE_FILE, "r") as f:
  #   reader = csv.DictReader(f)
  #   for row in reader:
  #     name = row["File"]
  #     bit = parse_int(row["Image Quality"])
  #     images.append((name, bit))

  # convert
  for name, bit in images:
    input_file = INPUT_DIR + name + ".NEF"
    convert_image(input_file, bit, CONVERT_WIDTH, CONVERT_HEIGHT)

    # move converted files to 
    move_image(input_file + CONVERT_SUFFIX, OUTPUT_DIR)



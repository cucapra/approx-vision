import subprocess
import csv

RAISE_FILE = "/approx-vision/RAW_RAISE/data/RAISE_all.csv"
INPUT_DIR = "/approx-vision/RAW_RAISE/data/dataset/"
OUTPUT_DIR = "/approx-vision/RAW_RAISE/data/dataset_converted/"
CONVERT_SCRIPT = "/approx-vision/RAW_RAISE/src/conversion/RawToPng"
CONVERT_DIR = "/approx-vision/RAW_RAISE/src/conversion/"

def execute(command):
  subprocess.call(" ".join(command), shell=True)

def convert_image(input_file, output_file, num_bits):
  command = [CONVERT_SCRIPT, input_file, output_file, num_bits]
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
  #     bit = str(parse_int(row["Image Quality"]))
  #     images.append((name, bit))

  # convert
  for name, bit in images:
    input_file = INPUT_DIR + name + ".NEF"
    output_file = OUTPUT_DIR + name + ".png"
    convert_image(input_file, output_file, bit)




import csv
import random
import re
import math
import pdb

RAISE_FILE = "../data/RAISE_all.csv"
OUTPUT_DIR = "../data/dataset_split/"
TRAIN_RATIO = 0.8

def analyze_categories(categories):
  mapping = {}

  for c in categories:
    mapping[c] = {}
    for cc in categories:
      mapping[c][cc] = 0

  # analyze category sets
  with open(RAISE_FILE, "r") as f:
    reader = csv.DictReader(f)
    for row in reader:
      keywords = row["Keywords"]
      keywords = keywords.split(";")
      keywords = list(map(lambda x: str.lower(str.strip(x)), keywords))

      for k in keywords:
        for kk in keywords:
          mapping[k][kk] += 1

  print(mapping)

def split_by_categories(tasks):
  overlap = {}
  files = [{} for i in tasks]

  # split files into task categories
  with open(RAISE_FILE, "r") as f:
    reader = csv.DictReader(f)
    for row in reader:
      keywords = row["Keywords"]
      keywords = keywords.split(";")
      keywords = list(map(lambda x: str.lower(str.strip(x)), keywords))

      name = row["File"]
      bit = parse_int(row["Image Quality"])

      for i, task in enumerate(tasks):
        task = set(task)
        filelist = files[i]
        common = list(task.intersection(keywords))
        if len(common) == 1:
          c = common[0]
          if not c in filelist:
            filelist[c] = []
          filelist[c].append(name + "," + bit) 
        else:
          if not str(common) in overlap:
            overlap[str(common)] = 0
          overlap[str(common)] += 1

  # save to files
  for i, task in enumerate(tasks):
    filelist = files[i]
    for c in task:
      with open(OUTPUT_DIR + dataset_file_name(task, c), "w") as outfile:
        for filename in filelist[c]:
          outfile.write(filename + '\n')

def split_into_train_test(tasks, train_ratio):
  for task in tasks:
    filename_train = dataset_file_name(task, ".train")
    filename_test = dataset_file_name(task, ".test")

    file_train = open(OUTPUT_DIR + filename_train, "w")
    file_test = open(OUTPUT_DIR + filename_test, "w")

    for c in task:
      filename = dataset_file_name(task, c)
      with open(OUTPUT_DIR + filename, "r") as f:
        lines = f.readlines()
        total = len(lines)
        num_train = int(math.floor(total * train_ratio))
        num_test = total - num_train
        isTrain = [True] * num_train
        isTrain.extend([False] * num_test)

        # shuffle data
        random.shuffle(isTrain)

        # split data
        for i, line in enumerate(lines):
          output_line = line.strip().split(",")
          output_line[1] = c
          output_line = ",".join(output_line)
          output_line += "\n"
          if isTrain[i]:
            file_train.write(output_line)
          else:
            file_test.write(output_line)

    file_train.close()
    file_test.close()
  return

def parse_int(s):
  return re.search(r'\d+', s).group()

def dataset_file_name(task, suffix):
  return "(" + "-".join(task) + ")" + suffix

if __name__ == "__main__":
  categories = ["outdoor", "indoor", "landscape", "nature", "people", "objects", "buildings"]
  tasks = [["outdoor", "indoor"], ["nature", "people", "objects"]]

  # analyze_categories(categories)
  split_by_categories(tasks)
  split_into_train_test(tasks, TRAIN_RATIO)
  



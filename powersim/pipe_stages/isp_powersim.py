#! /usr/bin/env python

from subprocess import call
import sys

if (len(sys.argv)<2):
  print "Need to provide stage to analyze in command line"
  print "Correct usage:"
  print "python isp_powersim.py ./executable" 
  sys.exit()

# Receive which executable to profile from command line
execute = sys.argv[1]

prof_cmds = [
  "perf stat -e cycles,instructions,cache-references,cache-misses ",
  "perf stat -e r532010 -e r534010 -e r538010 -e r531010 ",
  "perf stat -e r530110 -e r1570414 "
  ]

# Profile the cycles, instructions, cache access, cache misses, 
# and execution time
call([prof_cmds[0]+execute],shell=True)

# Profile the SSE single precision scalar, SSE single precision packed,
# SSE double precision scalar, and SSE double precision packed
call([prof_cmds[1]+execute],shell=True)

# Profile the x87 FP operations and FP divides
call([prof_cmds[2]+execute],shell=True)


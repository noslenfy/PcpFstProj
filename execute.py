#!/usr/bin/python

import sys
import subprocess

matrix = sys.argv[1]
lines = sys.argv[2]
columns = sys.argv[3]
total=0
n_times=5

n_threads = ['1','2','4','8','16','24']

for thread in n_threads:
  for i in range(0,n_times):
    proc = subprocess.Popen(["./project1",matrix,lines,columns,thread], stdout=subprocess.PIPE)
    output = proc.stdout.read()
    print "%d - execution with %s threads took: %s" % (i+1, thread, output);
    total += float(output)
  
  print "\nAverage execution with %s threads took: %f\n" % (thread, (total/n_times))
  total=0

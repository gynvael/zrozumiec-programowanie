#!/usr/bin/python
# -*- coding: utf-8 -*-
from glob import glob
import os
import stat
import sys

def list_files(pattern):
  print("--- Listing files for: %s" % pattern)

  # Alternatywnie możnaby użyć os.listdir.
  for entry in glob(pattern):
    mode = os.lstat(entry).st_mode
    t = "FILE"
    if stat.S_ISDIR(mode):
      t = "DIR "
    if stat.S_ISLNK(mode):
      # W przypadku linków w systemie Windows powyższy warunek mimo wszystko
      # nie będzie spełniony.
      t = "LINK"
    print("[%s] %s" % (t, os.path.basename(entry)))

def main():
  dirs = []
  if len(sys.argv) == 1:
    dirs.append("./*")
  else:
    dirs.extend(sys.argv[1:])

  for d in dirs:
    list_files(d)

if __name__ == "__main__":
  main()


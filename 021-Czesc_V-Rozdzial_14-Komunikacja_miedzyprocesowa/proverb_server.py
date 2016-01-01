#!/usr/bin/python
# -*- coding: utf-8 -*-
import os
import time

# Lista przysłów i powiedzeń ludowych zasłyszanych w internecie.
proverbs = [
    "Using floats in a banking app is INFINITE fun.",
    "threads.synchronize your Remember to ",
    "C and C++ are great for parsing complex formats [a hacker on IRC]",
    ( "There are only two hard things in CS: cache invalidation, naming things "
      "and off by one errors." )  # Phil Karlton + anonymous
    ]

# Stwórz nazwany potok.
try:
  os.mkfifo("/tmp/proverb", 0o644)
except OSError as e:
  if e.errno != 17:  # File exists
    raise

# Zajmij końcówkę zapisu i oczekuj na "połączenia" na końcówcę odczytu.
print ("Pipe /tmp/proverb created. Waiting for clients.")
print ("Press Ctrl+C to exit.")
proverb = 0
try:
  while True:
    fdw = open("/tmp/proverb", "w")
    print ("Client connected! Sending a proverb.")
    fdw.write(proverbs[proverb % len(proverbs)] + "\n")
    fdw.close()
    proverb += 1
    time.sleep(0.1)  # Daj chwilę czasu klientowi na rozłączenie się.
except KeyboardInterrupt:
  pass

# Usuń nazwany potok z systemu plików.
print ("\nCleaning up!")
os.unlink("/tmp/proverb")


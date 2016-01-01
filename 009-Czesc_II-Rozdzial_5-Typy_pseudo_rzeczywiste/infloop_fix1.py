#!/usr/bin/python
x = 0.0
epsilon = 0.000000000000001
while not (abs(x - 1.0) < epsilon):
  print x,
  x += 0.1


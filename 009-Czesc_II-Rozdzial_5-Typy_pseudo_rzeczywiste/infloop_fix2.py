#!/usr/bin/python
def fcmp(a, b):
  if a == b:
    return 0.0

  if abs(a) > abs(b):
    return abs(a - b) / a

  return abs(a - b) / b

x = 0.0
while not (fcmp(x, 1.0) < 0.0001):
  x += 0.1
  print x, fcmp(x, 1.0), fcmp(1.0, x)  



#!/usr/bin/python
k = [ 0.0, 0.1, 0.2, 0.3, 0.4 ,0.5, 0.6, 0.7, 0.8, 0.9, 1.0]
x = 0.0
dx = 0.1
for ki in k:
  print ki, x,  x == ki, "\t %.20f %.20f" % (ki, x)
  x += dx


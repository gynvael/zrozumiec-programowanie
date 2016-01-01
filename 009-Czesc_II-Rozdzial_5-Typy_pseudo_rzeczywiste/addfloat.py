import timeit

# Note: Subnormal numbers begin around e-308 or p-1022
for e in xrange(301, 324):
  x = float("1.234e-%i" % e)
  t = timeit.Timer(lambda: x * 2.1)
  print x.hex(), t.timeit(number = 10000000)


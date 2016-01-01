from struct import pack, unpack
import sys
import math

if __name__ == "__main__":
  for x in sys.argv[1:]:
    x = float(x)

    # Encode to bytes and reinterpret as uint64.
    x_bytes = pack(">d", x)
    x_uint64, = unpack(">Q", x_bytes)

    # Do standard bit operations to parts of the double.
    # Note: ((1 << N) - 1) creates a bit pattern of N ones.
    x_sign = (x_uint64 >> 63) & 1
    x_exponent = (x_uint64 >> 52) & ((1 << 11) - 1)
    x_fraction = x_uint64 & ((1 << 52) - 1)

    # Display.
    x_fraction_str = bin(x_fraction)[2:].ljust(52, "0")
    print "--- Double Precission Number:", x
    print "Hex : %.16x" % x_uint64
    print "Sign: %s" % "+-"[x_sign]
    print "Exp : %u (dec)" % (x_exponent - 1023)
    print "Frac: 1.%s (bin)\n" % x_fraction_str

  

#!/usr/bin/python
# -*- coding: utf-8 -*-
import struct
import sys

def manual_decode_utf16le(s):
  unicodes = []
  code = None  
  i = 0
  while i < len(s):
    # Odczytaj 16-bitową liczbę naturalną (LE).    
    codepoint = struct.unpack("<H", s[i:i+2])[0]

    if ((codepoint >= 0x0000 and codepoint <= 0xD7FF) or
        (codepoint >= 0xE000 and codepoint <= 0xFFFF)):
      unicodes.append(codepoint)
      i += 2      
      continue

    if code is None:
      # Mamy do czynienia w pierwszą 16-bitową liczbą. Warto się upewnić czy
      # tak na pewno jest.
      if not (codepoint >= 0xD800 and codepoint <= 0xDBFF):
        print>>sys.stderr, "Invalid UTF-16 string (1)."
        return None
      code = (codepoint & 0x3ff) << 10
      i += 2
      continue

    # Mamy do czynienia z drugą 16-bitową liczbą. Podobnie jak powyżej, warto
    # sprawdzić czy tak na pewno jest.
    if not (codepoint >= 0xDC00 and codepoint <= 0xDFFF):
      print>>sys.stderr, "Invalid UTF-16 string (2)."
      return None

    code |= (codepoint & 0x3ff)
    code += 0x10000
    unicodes.append(code)
    code = None
    i += 2    
    
  return unicodes

s = u"Piktogram kota: \U0001f408"
sutf16 = s.encode("utf-16-le")
for ch in manual_decode_utf16le(sutf16):
  print "u+%x " % ch,
print ""




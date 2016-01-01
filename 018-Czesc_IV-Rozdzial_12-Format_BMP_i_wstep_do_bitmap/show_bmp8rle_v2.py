#!/usr/bin/python
# -*- coding: utf-8 -*-
import array
import pygame
from pygame.locals import *
import struct

import sys


def MyLoadBMP(filename):
  # Read the whole file into a bufffer.
  with open(filename, "rb") as f:
    data = f.read()

  if data[:2] != 'BM':
    # Not a BMP file.
    return None

  # Unpack BITMAPFILEHEADER.
  bfType, bfSize, bfRes1, bfRes2, bfOffBits = struct.unpack("<HIHHI", data[:14])

  # Unpack BITMAPINFOHEADER.
  (biSize, biWidth, biHeight, biPlanes, biBitCount, biCompression, biSizeImage,
   biXPelsPerMeter, biYPelsPerMeter, biClrUser, biClrImportant) = struct.unpack(
       "<IIIHHIIIIII", data[14:14 + 40])

  if biSize != 40:
    # Unknown BMP type.
    return None

  if biBitCount == 24 and biCompression == 0:  # BI_RGB
    return MyLoadBMP_RGB24(data, bfOffBits, biWidth, biHeight)
  elif biBitCount == 8 and biCompression == 1:  # BI_RLE8
    return MyLoadBMP_RLE8(data, bfOffBits, biWidth, biHeight, biClrUser)

  # Unsupported BMP type.
  return None


def MyLoadBMP_RLE8(data, pixel_offset, w, h, clr_used):
  # Czy wiersze są zapisane od dołu do góry?
  bottom_up = True
  if h < 0:
    bottom_up = False
    h = - h

  PALETTE_OFFSET = 0x0e + 0x28  # Wielkości obu nagłówków.
  palette = [ord(b) for b in data[PALETTE_OFFSET:PALETTE_OFFSET + clr_used * 4]]

  # Zaalokuj tablicę (bufor) na 24-bitową bitmapę BGR. Wypełnij ją kolorem tła
  # (tj. pierwszym wpisem z palety kolorów).
  bitmap = array.array('B', palette[:3]) * (w * h)

  # Wczytaj opkod RLE.
  if bottom_up:
    r = xrange(h - 1, -1, -1)
  else:
    r = xrange(0, h)

  r = iter(r)  # Pobierz iterator wierszy.
  d = iter([ord(b) for b in data[pixel_offset:]])  # Pobierz iterator danych.

  # Uwaga: Poniżej nie ma praktycznie żadnego sprawdzania błędów.
  x = 0
  y = r.next()
  while True:
    rle_opcode = d.next()

    if rle_opcode > 0:  # Normalna kompersja RLE.
      repeat_count = rle_opcode
      palette_index = d.next()
      pixel = palette[palette_index * 4:palette_index * 4 + 4]
      if len(pixel) == 0:
        # Uznaj kolor za czarny jeśli indeks nie mieści się w palecie kolorów.
        pixel = [0, 0, 0, 0]      

      for _ in xrange(repeat_count):
        bitmap[(x + y * w) * 3 + 0] = pixel[0]
        bitmap[(x + y * w) * 3 + 1] = pixel[1]
        bitmap[(x + y * w) * 3 + 2] = pixel[2]
        x += 1
      continue

    # Opkod jest równy 00 - pobierz opkod rozszerzający.
    rle_opcode = d.next()
    if rle_opcode == 0:  # 00 00 - Koniec wiersza..
      x = 0
      y = r.next()
    elif rle_opcode == 1:  # 00 01 - Koniec bitmapy.
      break
    elif rle_opcode == 2:  # 00 02 XX YY - Przesuń kursor zapisu.
      x += d.next()

      # Pobierz YY numerów wierszy.
      for _ in xrange(d.next()):
        y = r.next()

    else:  # 00 NN ... - Surowe dane.
      raw_count = rle_opcode
      for _ in xrange(raw_count):
        palette_index = d.next()
        pixel = palette[palette_index * 4:palette_index * 4 + 4]
        if len(pixel) == 0:
          # Uznaj kolor za czarny jeśli indeks nie mieści się w palecie kolorów.
          pixel = [0, 0, 0, 0]

        bitmap[(x + y * w) * 3 + 0] = pixel[0]
        bitmap[(x + y * w) * 3 + 1] = pixel[1]
        bitmap[(x + y * w) * 3 + 2] = pixel[2]
        x += 1

      if raw_count % 2 != 0:  # Bajt paddingu?
        d.next()

  return (w, h, 24, bitmap)


def MyLoadBMP_RGB24(data, pixel_offset, w, h):
  # Up-side down?
  bottom_up = True
  if h < 0:
    bottom_up = False
    h = - h

  # Figure out pitch.
  pitch = (w * 3 + 3) & ~3

  # Allocate array (24-bit, color order: BGR).
  bitmap = array.array('B', [0]) * w * h * 3

  # Read rows.
  if bottom_up:
    r = xrange(h - 1, -1, -1)
  else:
    r = xrange(0, h)

  for y in r:
    for x in xrange(0, w):
      bitmap[(x + y * w) * 3 + 0] = ord(data[pixel_offset + x * 3 + 0])
      bitmap[(x + y * w) * 3 + 1] = ord(data[pixel_offset + x * 3 + 1])
      bitmap[(x + y * w) * 3 + 2] = ord(data[pixel_offset + x * 3 + 2])
    pixel_offset += pitch

  return (w, h, 24, bitmap)

# Create a 640x480x24 window.
WINDOW_W = 640
WINDOW_H = 480
pygame.display.init()
window = pygame.display.set_mode([WINDOW_W, WINDOW_H], 0, 24)

# Create a black-red gradient raw bitmap in a buffer.
image_w, image_h, image_bpp, image_data = MyLoadBMP("test8rle.bmp")

# "Manually" copy rows from raw bitmap to pygame framebuffer.
pixels = pygame.PixelArray(window)
center_x = (WINDOW_W - image_w) / 2
center_y = (WINDOW_H - image_h) / 2
for y in xrange(image_h):
  for x in xrange(image_w):
    pixel = image_data[(x + y * image_w) * 3:(x + y * image_w) * 3 + 3]
    pixel = pixel[0] | (pixel[1] << 8) | (pixel[2] << 16)
    pixels[center_x + x, center_y + y] = pixel

# Make PyGame and the OS flush the PyGame screen buffer to the actual screen.
pygame.display.flip()

# Wait until the window is closed or ESC is pressed.
while True:
  event = pygame.event.wait()
  if event.type == KEYDOWN and event.key == K_ESCAPE:
    break
  if event.type == QUIT:
    break
pygame.quit()


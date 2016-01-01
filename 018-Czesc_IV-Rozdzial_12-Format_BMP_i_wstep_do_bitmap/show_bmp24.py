#!/usr/bin/python
# -*- coding: utf-8 -*-
import array
import pygame
from pygame.locals import *
import struct

def MyLoadBMP(filename):
  # Wczytaj cały dplik do bufora.
  with open(filename, "rb") as f:
    data = f.read()

  if data[:2] != 'BM':
    # Nieprawidłowy plik BMP.
    return None

  # Rozkoduj BITMAPFILEHEADER.
  bfType, bfSize, bfRes1, bfRes2, bfOffBits = struct.unpack("<HIHHI", data[:14])

  # Rozkoduj BITMAPINFOHEADER.
  (biSize, biWidth, biHeight, biPlanes, biBitCount, biCompression, biSizeImage,
   biXPelsPerMeter, biYPelsPerMeter, biClrUser, biClrImportant) = struct.unpack(
       "<IIIHHIIIIII", data[14:14 + 40])

  if biSize != 40:
    # Nieobsługiwany wariant BMP.
    return None

  if biBitCount == 24 and biCompression == 0:  # BI_RGB
    return MyLoadBMP_RGB24(data, bfOffBits, biWidth, biHeight)

  # Nieobsługiwane kodowanie.
  return None

def MyLoadBMP_RGB24(data, pixel_offset, w, h):
  # Czy wiersze są zapisane od dołu do góry?
  bottom_up = True
  if h < 0:
    bottom_up = False
    h = - h

  # Oblicz pitch.
  pitch = (w * 3 + 3) & ~3

  # Stwórz nowy bufor na odczytaną bitmapę (24 BPP, kolejność kolorów: BGR).
  bitmap = array.array('B', [0]) * w * h * 3

  # Wczytaj wiersze.
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

# Stwórz okno o wielkości 640x480 i 24 BPP.
WINDOW_W = 640
WINDOW_H = 480
pygame.display.init()
window = pygame.display.set_mode([WINDOW_W, WINDOW_H], 0, 24)

# Wczytaj testową bitmapę.
image_w, image_h, image_bpp, image_data = MyLoadBMP("test.bmp")

# „Manualnie” skopiuj wiersze wczytanej bitmapy do bufora klatki pygame.
pixels = pygame.PixelArray(window)
center_x = (WINDOW_W - image_w) / 2
center_y = (WINDOW_H - image_h) / 2
for y in xrange(image_h):
  for x in xrange(image_w):
    pixel = image_data[(x + y * image_w) * 3:(x + y * image_w) * 3 + 3]
    pixel = pixel[0] | (pixel[1] << 8) | (pixel[2] << 16)
    pixels[center_x + x, center_y + y] = pixel

# Przerysuj ekran (tj. wyświetl bufor klatki po którym rysowaliśmy).
pygame.display.flip()

# Poczekaj aż okno zostanie zamknięte lub zostanie naciśnięty przycisk ESC.
while True:
  event = pygame.event.wait()
  if event.type == KEYDOWN and event.key == K_ESCAPE:
    break
  if event.type == QUIT:
    break
pygame.quit()


#!/usr/bin/python
# -*- coding: utf-8 -*-
import pygame
from pygame.locals import *
import array

# Stwórz okno o wielkości 640x480 i 24 BPP.
WINDOW_W = 640
WINDOW_H = 480
pygame.display.init()
window = pygame.display.set_mode([WINDOW_W, WINDOW_H], 0, 24)

# Narysuj czarno-czerwony gradient w oddzielnym buforze.
W = 256
H = 256
gradient = array.array('L', [0]) * W * H  # Array 
for y in xrange(H):
  for x in xrange(W):
    gradient[x + y * W] = (x & 0xff)  # Red=x, Green=0, Blue=0

# „Manualnie” skopiuj wiersze z powyższej tablicy do bufora klatki pygame.
pixels = pygame.PixelArray(window)
center_x = (WINDOW_W - W) / 2
center_y = (WINDOW_H - H) / 2
for y in xrange(H):
  for x in xrange(W):
    # PyGame używa formatu BGR, w buforze z gradientem celowo użyłem RGB.
    # Poniżej dokonywana jest konwersja RGB do BGR (zakładam, że 'L' w 
    # array.array jest zapisane little-endian).
    pixel = gradient[x + y * W]
    pixel = (((pixel & 0x0000ff) << 16) |  # R jest przesuwane na górny bajt.
             (pixel & 0x00ff00) |          # G pozostaje bez zmian.
             ((pixel & 0xff0000) >> 16))   # B jest przesuwane na dolny bajt.
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


#!/usr/bin/python
# -*- coding: utf-8 -*-
from struct import pack

# Stwórz plik w postaci:
# N
# 0:
# 1: 1
# 2: 2 2
# ...
# N-1: N-1 N-1 ... N-1

N = 5
data = []

# Dopisz do danych 16-bitową ilość zestawów.
# Znak < oznacza Little Endian, natomiast H 16-bitową liczbę naturalną.
data.append(pack("<H", N))

# Wygeneruj N zestawów z prostymi liczbami.
for M in range(N):
  # Dopisz 8-bitową liczbę z ilością liczb w zestawie.
  # Znak B oznacza 8-bitową liczbę naturalną.
  data.append(pack("B", M))

  # Dopisz M wartości w zestawie.
  # Znak I oznacza 32-bitową liczbę naturalną.
  for _ in range(M):
    data.append(pack("<I", M))

# Zapisz wszystko do pliku.
with open("example.bin", "wb") as f:
  f.write(b''.join(data))


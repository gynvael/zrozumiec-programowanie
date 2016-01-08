#!/usr/bin/python
# watch pep8 --show-pep8 --ignore=E111,E114,E241,W391 ./vm.py


class VMMemory(object):
  def __init__(self):
    self._mem = bytearray(64 * 1024)

  def fetch_byte(self, addr):
    if addr < 0 or addr >= len(self._mem):
      return None

    return self._mem[addr]

  def store_byte(self, addr, value):
    if addr < 0 or addr >= len(self._mem):
      return False
    self._mem[addr] = value
    return True

  def fetch_dword(self, addr):
    if addr < 0 or addr + 3 >= len(self._mem):
      return None
    return (self._mem[addr] |
            (self._mem[addr + 1] << 8) |
            (self._mem[addr + 2] << 16) |
            (self._mem[addr + 3] << 24))

  def store_dword(self, addr, value):
    if addr < 0 or addr + 3 >= len(self._mem):
      return False
    self._mem[addr] = value & 0xff
    self._mem[addr + 1] = (value >> 8) & 0xff
    self._mem[addr + 2] = (value >> 16) & 0xff
    self._mem[addr + 3] = (value >> 24) & 0xff
    return True

  def fetch_many(self, addr, size):
    if addr + size - 1 >= len(self._mem):
      return None
    return self._mem[addr:addr + size]

  def store_many(self, addr, array):
    if addr + len(array) - 1 >= len(self._mem):
      return False
    for i, value in enumerate(array):
      self._mem[addr + i] = value
    return True

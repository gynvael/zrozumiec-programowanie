#!/usr/bin/python
# -*- coding: utf-8 -*-
from struct import unpack
from zlib import crc32

# Niewielka ale bardzo pomocna klasa do odczytu danych z buforu.
class StreamReader:
  def __init__(self, data):
    self.offset = 0
    self.data = data

  def peek_block(self, size):
    return self.data[self.offset:self.offset + size]

  def get_block(self, size):
    b = self.data[self.offset:self.offset + size]
    self.offset += size
    return b

  def get_uint32(self):
    return unpack(">I", self.get_block(4))[0]

  def get_uint16(self):
    return unpack(">H", self.get_block(2))[0]

  def get_uint8(self):
    return ord(self.get_block(1))

# Wyjątek rzucany w przypadku błędu w dekodowaniu.
class PNGError(Exception):
  pass

class PNGReader:
  def __init__(self, fname):
    self.fname = fname
    self.png = None
    self.bitmap_data = []
    self.header = { }
    self.bitmap = {
        "header": self.header,
        "rgb": [ ]
        }

  def _verify_magic(self):
    PNG_HEADER = "\x89PNG\r\n\x1a\n"
    return PNG_HEADER == self.png.get_block(len(PNG_HEADER))

  def _read_chunk(self):
    # Wczytaj pojedynczy blok.
    chunk = { }
    chunk["length"] = self.png.get_uint32()
    chunk["type"]   = self.png.get_block(4)
    chunk["data"]   = self.png.get_block(chunk["length"])
    chunk["crc32"]  = self.png.get_uint32()

    # Wylicz CRC32 z type i data.
    crc = crc32(chunk["type"])
    crc = crc32(chunk["data"], crc) & 0xffffffff

    if chunk["crc32"] != crc:
      raise PNGError("chunk %s CRC32 incorrect" % chunk["type"])

    return chunk

  def _process_IHDR(self, chunk):
    # Wczytaj pola nagłówka.
    data = StreamReader(chunk["data"])
    self.header["width"]       = data.get_uint32()
    self.header["height"]      = data.get_uint32()
    self.header["bpp"]         = data.get_uint8()
    self.header["color"]       = data.get_uint8()
    self.header["compression"] = data.get_uint8()
    self.header["filter"]      = data.get_uint8()
    self.header["interlace"]   = data.get_uint8()

    # Ten dekoder obsługuje tylko PNG typu 24-bpp bez przeplotu. Sprawdź czy
    # dekodowany plik zawiera dokładnie taki PNG.
    if self.header["bpp"] != 8:
      raise PNGError("unsupported bpp (%u)" % self.header["bpp"])

    if self.header["color"] != 2:
      raise PNGError("unsupported color type (%u)" % self.header["color"])

    if self.header["compression"] != 0:
      raise PNGError("unsupported compression type (%u)" %
                     self.header["compression"])

    if self.header["filter"] != 0:
      raise PNGError("unsupported filter type (%u)" % self.header["filter"])

    if self.header["interlace"] != 0:
      raise PNGError("unsupported interlace type (%u)" %
                     self.header["interlace"])

    return

  def _process_IDAT(self, chunk):
    # Dodaj dane do bitmap_data. Zostaną zdekompresowane później.
    self.bitmap_data.append(chunk["data"])

  # Implementacja filtrów wg. dokumentacji PNG.
  def _paeth(self, a, b, c):
    p = a + b - c
    pa = abs(p - a)
    pb = abs(p - b)
    pc = abs(p - c)
    if pa <= pb and pa <= pc:
      return a
    elif pb <= pc:
      return b
    return c

  def _decode_row_none(self, row, off_x, prior, data):
    row.extend(data)

  def _decode_row_sub(self, row, off_x, prior, data):
    for x in xrange(len(data)):
      row.append((data[x] + row[off_x + x - 3]) & 0xff)

  def _decode_row_up(self, row, off_x, prior, data):
    for x in xrange(len(data)):
      row.append((data[x] + prior[off_x + x]) & 0xff)

  def _decode_row_average(self, row, off_x, prior, data):
    for x in xrange(len(data)):
      row.append((data[x] + (row[off_x + x - 3] + prior[off_x + x]) / 2) & 0xff)

  def _decode_row_paeth(self, row, off_x, prior, data):
    for x in xrange(len(data)):
      pred = self._paeth(row[off_x + x - 3], prior[off_x + x],
                         prior[off_x + x - 3])
      row.append((data[x] + pred) & 0xff)
  # Koniec filtrów.

  def _process_bitmap_data(self, data):
    # Dekompresja.
    data = StreamReader(data.decode("zlib"))
   
    # Chwilowo dodam cały wiersz z zerami. Również na początku każdego wiersza
    # będę dodawać po kilka zera - uprości to logikę filtrów (tj. nie będzie
    # konieczne sprawdzanie czy piksel z poprzedniego wiersza/kolumny istnieje.
    # Wg. dokumentacji taki piksel na potrzeby filtrów powinien mieć wartość 0.
    rows = []
    rows.append([0] * ((self.header["width"] + 1) * 3))
    off_y = 1  # Pozycja pierwszego wiersza prawdziwej bitmapy w rows.
    off_x = 3  # Pozycja pierwszego bajtu prawdziwej bitmapy w wierszu.

    # Obsługa poszczególnych filtrów.
    filter_handlers = [
        self._decode_row_none,
        self._decode_row_sub,
        self._decode_row_up,
        self._decode_row_average,
        self._decode_row_paeth,
        ]

    # Odwrócenie filtrów dla każdego wiersza.
    for y in xrange(self.header["height"]):
      filter_type = data.get_uint8()
      if filter_type >= len(filter_handlers):
        raise PNGError("invalid PNG filter")

      filtered_data = map(ord, data.get_block(self.header["width"] * 3))
      prior_data = rows[off_y + (y - 1)]  # Poprzedni wiersz.
      row_data = [0] * off_x  # Tablica na zdekodowane wartości.
      filter_handlers[filter_type](row_data, off_x, prior_data, filtered_data)
      rows.append(row_data)

    # Usunięcie dodanych wcześniej nadmiarowych zer.
    rows.pop(0)
    for r in rows:
      r.pop(0)
      r.pop(0)
      r.pop(0)      

    self.bitmap["rgb"].extend(rows)

  def decode(self):
    with open(self.fname, "rb") as f:
      self.png = StreamReader(f.read())

    # Sprawdzenie nagłówka.
    if not self._verify_magic():
      raise PNGError("incorrect magic")

    # Odczytanie i przetworzenie kolejnych bloków.
    # Uwaga: Niniejszy, przykładowy dekoder nie sprawdza czy bloki wystapiły
    # w określonej w dokumentacji ilości oraz kolejności.
    chunk_handlers = {
        "IHDR": self._process_IHDR,
        "IDAT": self._process_IDAT,
        }

    while True:
      chunk = self._read_chunk()
      if chunk["type"] == "IEND":
        break

      if chunk["type"] in chunk_handlers:
        chunk_handlers[chunk["type"]](chunk)
        continue

      print "warning: chunk %s not handled, ignoring" % chunk["type"]

    # Rozkoduj dane bitmapy.
    self._process_bitmap_data(''.join(self.bitmap_data))
    return self.bitmap


def main():
  png = PNGReader("test.png")
  b = png.decode()

  # Zapisz dane jako surową bitmapę.
  with open("dump.raw", "wb") as f:
    for row in b["rgb"]:
      f.write(''.join(map(chr, row)))


if __name__ == "__main__":
  main()


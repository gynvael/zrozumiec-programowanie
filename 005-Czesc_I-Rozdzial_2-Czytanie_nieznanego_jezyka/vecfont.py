import struct

class VecFont():
  def __init__(self):
    self.__gfx_engine = None
    self.__font = None

  def set_gfx_engine(self, gfx):
    self.__gfx_engine = gfx

  def load_font_from_string(self, font_string):
    # Note: The font string is a DEFLATed serialized array of glyphs. Each 
    # glyph is an array of paths. Each path is an array of coordinates.
    # This must be deserialized into Python arrays.
    font_string = font_string.decode("zlib")
    data = iter(font_string)
    
    # Read all letters.
    self.__font = []
    while True:
      number_of_paths = self.__read_uint8(data)
      if number_of_paths == 0:
        break

      paths = []
      for _ in xrange(number_of_paths):
        number_of_coords = self.__read_uint8(data)
        coords = []
        for _ in xrange(number_of_coords):
          coords.append((
              self.__read_float(data),   # X
              self.__read_float(data)))  # Y
        paths.append(coords)

      self.__font.append(paths)

  def render_string(self, text, sx, sy, font_size):
    character_x = sx
    character_y = sy

    for character in text:
      translate_x = self.render_char(
          ord(character), character_x, character_y, font_size)
      translate_x += int(0.1 * font_size)

      character_x += translate_x

  def render_char(self, ch, sx, sy, font_size):
    max_x = 0
    for paths in self.__font[ch]:
      last = None
      for coords in paths:
        if coords[0] > max_x: max_x = coords[0]

        if last == None:  # Skip first point.
          last = coords
          continue

        self.__gfx_engine.draw_line(
            int(sx + last[0] * font_size),
            int(sy + last[1] * font_size),
            int(sx + coords[0] * font_size),
            int(sy + coords[1] * font_size))

        last = coords
        
    return int(max_x * font_size)

  def __read_uint8(self, data):
    return ord(data.next())

  def __read_float(self, data):
    b = ""
    for _ in xrange(4):
      b += data.next()
    return struct.unpack(">f", b)[0]

def test_vec_font():
  FONT = ("eJxNUiFQw0AQTL6BTFUcMg4Zh0z+BQJR0RkEIq6yghlkZR04XOtwrcO1Dp"
          "c6XOpwqcMVh2R381+amZ9NLrd3t3tvBhEe93T8ifxjj/27KRHkh91stsQ4"
          "ic4e91gU9rbZCef4ruvaDCvkqshbngubZmfT5xfharVm3HbdoWrbPVE570"
          "VBrBaLZeAn9nM0EoH4PZ2G40hAzAxZ9pzOaMVxA5Pnq67VCqgcxNSK0jz/"
          "wjIBRYg0wr5mmYMiFhPebLYOSuPU3rd7lQFVrajsoywVoyoy/D83whjXq7"
          "WbdIdYJstPHDkWTB6ebEfX4JhGpmN0iyzU5hZclmXCO/RlpUnvINFwCUws"
          "PSG+DGqDUin3jVlGEugr2wBNEjI4gAqXvTj3gNUgZpimBlzFFJT/NWBe4e"
          "9spvmBbox54Q8zGSOq3LjXqjzPN3xhQihkTtctTGOxCFlOhMKE989eLZY2"
          "jJv1dV2e51xxnJb+YmtjfunaNe4aL4WaDvo7GqaSq38Mnx3i").decode("base64")

  vf = VecFont()
  vf.load_font_from_string(FONT)
  vf.set_gfx_engine(TestGraphicsEngine())
  vf.render_string("\0\1\2\3\4\5\6\7\x08\x09\x0a\x0b", 100, 100, 70)


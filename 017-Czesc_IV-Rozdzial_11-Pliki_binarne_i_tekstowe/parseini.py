#!/usr/bin/python
# -*- coding: utf-8 -*-
import pprint
import sys


def parse_ini(fname):
  ini = {}
  section = {}
  ini["__global__"] = section

  with open(fname) as f:
    for ln in f:  # Odczyt linia po linii.
      if ln.startswith(';'):
        continue  # Komentarz

      # Usuń białe znaki z końca/początku linii.
      ln = ln.strip()

      if ln.startswith('[') and ln.endswith(']'):
        section_name = ln[1:-1].strip()  # Wydobądź nazwę sekcji.
        section = {}
        ini[section_name] = section
        continue

      if '=' not in ln:
        continue  # Ignorowanie pustych i niepoprawnych linii.

      # Rozbij linię na dwie części względem znaku = i usuń białe znaki
      # z poczatku i końca obu uzyskanych części.
      name, val = [x.strip() for x in ln.split("=", 1)]
      section[name] = val

  return ini


if __name__=="__main__":
  pp = pprint.PrettyPrinter()
  pp.pprint(parse_ini("test.ini"))


#!/usr/bin/python
# -*- coding: utf-8 -*-
import glob

print "%5s  %-15s  %-12s  %s" % ("PID", "Name", "State", "Command line")
print "-" * 60

# Dla każdego wpisu w katalogu proc...
for name in glob.glob("/proc/*"):

  # Sprawdź czy wpis składa się tylko z cyfr.
  pid = name.replace("/proc/", "")
  if not pid.isdigit():
    continue  # Inny plik lub katalog.
  pid = int(pid)

  # Wczytaj linię poleceń z pliku cmdline. Kolejne argumenty są oddzielone
  # zerem binarnym.
  with open("/proc/%i/cmdline" % pid) as f:
    cmdline = f.read().replace("\0", " ")
  
  # Wczytaj garść informacji z pliku status.
  info = {}
  for line in open("/proc/%i/status" % pid):
    data = line.strip().split(":\t")
    if len(data) == 2:
      info[data[0]] = data[1].strip()

  # Wypisz informacje o procesie.
  print "%5u  %-15s  %-12s  %s" % (pid, info["Name"], info["State"], cmdline)





#!/usr/bin/python
# -*- coding: utf-8 -*-
import os
import subprocess

# W przypadku niepowodzenia przy tworzeniu procesu jest rzucany odpowiedni
# wyjątek, zależny niestety od systemu. Przykładowo, jeśli plik nie zostanie
# znaleziony, na systemach z rodziny Windows zostanie rzucony wyjątek
# WindowsError, ale na systemach z rodziny GNU/Linux rzucony będzie OSError.
if os.name == 'nt':
  exception_type = WindowsError
  executable = "C:\\Windows\\System32\\notepad.exe"
else:
  exception_type = OSError
  executable = "/usr/bin/vim"

try:
  p = subprocess.Popen(executable)
  print "Process started (pid: %u). Waiting for it to exit." % p.pid
  p.wait()
  print "The child process has exited."
except exception_type as e:
  print "Failed to spawn process:", e

# Alternatywne funkcje z moduły subprocess, które oczekują na zakończenie
# procesu dziecka (co oznacza, że nie jest zwracany "uchwyt" do procesu
# dziecka).
#
# subprocess.call(["C:\\Windows\\System32\\Notepad.exe"])
# subprocess.check_call(["C:\\Windows\\System32\\Notepad.exe"])
# subprocess.call_output(["C:\\Windows\\System32\\Notepad.exe"])
# subprocess.check_output(["C:\\Windows\\System32\\Notepad.exe"])
#


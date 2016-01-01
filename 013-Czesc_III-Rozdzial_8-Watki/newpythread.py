#!/usr/bin/python
# -*- coding: utf-8 -*-
import threading

class MyThread(threading.Thread):
  def __init__(self, data):
    super(MyThread, self).__init__()    
    self.__data = data
    self.__retval = None

  def run(self):
    print("I was run in a new thread with %x as data." % self.__data)
    self.__retval = 0xC0DE

  def get_retval(self):
    return self.__retval

# Stworzenie nowego wątku.
th = MyThread(0x12345678)
th.start()

# Oczekiwanie na zakończenie nowego wątku.
th.join()
print("Second thread returned: %x" % th.get_retval())


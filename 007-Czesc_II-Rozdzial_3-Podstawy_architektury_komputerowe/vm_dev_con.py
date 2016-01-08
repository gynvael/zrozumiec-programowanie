#!/usr/bin/python
# watch pep8 --show-pep8 --ignore=E111,E114,E241,W391 ./vm.py
import sys
import time
import threading


class ConsoleWorker(threading.Thread):
  def __init__(self, console_dev):
    super(ConsoleWorker, self).__init__()
    self.dev = console_dev

    self.queue = []
    self.queue_mutex = threading.Lock()

    self.shutdown = threading.Event()

  def run(self):
    while not self.shutdown.is_set():
      ch = sys.stdin.read(1)
      if ch == "":
        # Seems stdin was closed. There will never be any more data.
        break

      with self.queue_mutex:
        self.queue.append(ord(ch))

      self.dev.new_data_ready()  # Notify the parent class.

  def get_character(self):
    ch = ""
    with self.queue_mutex:
      if len(self.queue) > 0:
        ch = self.queue.pop(0)
    return ch

  def data_ready(self):
    with self.queue_mutex:
      res = len(self.queue) > 0
    return res


class VMDeviceConsole():
  def __init__(self, vm):
    self.vm = vm
    self.worker = ConsoleWorker(self)
    self.worker.start()

    self.control_register_mutex = threading.Lock()
    self.control_register = 0

  def new_data_ready(self):
    # Called by worker.
    self.control_register_mutex.acquire()
    if self.control_register & 1:
      self.control_register &= 0xfffffffe
      self.control_register_mutex.release()
      self.vm.interrupt(9)
    else:
      self.control_register_mutex.release()

  def handle_inbound(self, port, byte):
    if port == 0x20:
      sys.stdout.write(chr(byte))
      sys.stdout.flush()
    elif port == 0x21:
      pass  # Ignored.
    elif port == 0x22:
      self.control_register_mutex.acquire()
      self.control_register = byte
      self.control_register_mutex.release()

  def handle_outbound(self, port):
    if port == 0x20:
      while True:
        ch = self.worker.get_character()
        if ch != "":
          break
        time.sleep(0)
      return ch
    elif port == 0x21:
      return int(self.worker.data_ready())
    elif port == 0x22:
      with self.control_register_mutex:
        res = self.control_register
      return res

  def terminate(self):
    self.worker.shutdown.set()
    # Since there is no way in Python to break waiting on read from stdin,
    # one cannot wait for the stdin thread. Instead this version kills all
    # threads (implicitly) at the end.
    # self.worker.join()

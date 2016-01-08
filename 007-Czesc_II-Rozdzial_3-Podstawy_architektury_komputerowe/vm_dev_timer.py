#!/usr/bin/python
# watch pep8 --show-pep8 --ignore=E111,E114,E241,W391 ./vm.py
import threading
import time


class TimerWorker(threading.Thread):
  def __init__(self, vm):
    super(TimerWorker, self).__init__()
    self.vm = vm

    self.mutex = threading.Lock()
    self.alarm_time = 0
    self.activation_time = 0
    self.active = False
    self.shutdown = threading.Event()

    # This variable is used only in one thread (caller). No need to lock it.
    self.alarm = 0

  def set_alarm(self, miliseconds):
    self.alarm = miliseconds

  def activate(self):
    with self.mutex:
      self.activation_time = time.time()
      self.alarm_time = self.activation_time + self.alarm / 1000.0
      self.active = True

  def deactivate(self):
    with self.mutex:
      self.active = False

  def get_counter(self):
    with self.mutex:
      # We don't really have to count.
      res = int((time.time() - self.alarm_time) * 1000)
    return res

  def run(self):
    while not self.shutdown.is_set():
      self.mutex.acquire()
      if self.active and time.time() >= self.alarm_time:
        self.active = False
        self.mutex.release()
        self.vm.interrupt(self.vm.INT_PIT)
      else:
        self.mutex.release()

      time.sleep(0)


class VMDeviceTimer():
  def __init__(self, vm):
    self.worker = TimerWorker(vm)
    self.worker.start()
    self.control_register = 0

    self.remaining_counter_value = 0
    self.has_counter_data = False

  def handle_inbound(self, port, byte):
    if port == 0x71:
      self.worker.alarm = ((self.worker.alarm << 8) | byte) & 0xffff
    elif port == 0x70:
      activation_bit = byte & 1
      if activation_bit == 1:
        self.control_register = 1
        self.worker.activate()
      else:
        self.control_register = 0
        self.worker.deactivate()

  def handle_outbound(self, port):
    if port == 0x70:
      return self.control_register
    elif port == 0x71:
      if self.has_counter_data is False:
        counter = self.worker.get_counter()
        self.remaining_counter_value = (counter >> 8) & 0xff
        self.has_counter_data = True
        return counter & 0xff
      else:
        self.has_counter_data = False
        return self.remaining_counter_value

  def terminate(self):
    self.worker.shutdown.set()
    self.worker.join()

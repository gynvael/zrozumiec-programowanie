#!/usr/bin/python
# -*- coding: utf-8 -*-
from threading import Condition, Lock, Thread
from time import sleep

class WorkContainer:
  def __init__(self):
    self.work = []
    self.mutex = Lock()
    self.cond = Condition(lock = self.mutex,
                          verbose = True)  # Wypisanie wew. informacji.
    self.work_complete = False

class ProducerThread(Thread):
  def __init__(self, work):
    super(ProducerThread, self).__init__()
    self.work = work

  def run(self):
    # Wygeneruj nowe dane.
    for i in xrange(16):
      sleep(0.15)  # Przygotowanie danych trwa dlugo :)
      new_data = i

      # Dodaj nowy pakiet danych do listy i wybudź jeden wątek.
      self.work.mutex.acquire()  # Tożsame z self.cond.acquire().
      self.work.work.append(new_data)
      self.work.cond.notify(1)  # Wybudź jeden wątek.
      self.work.mutex.release()

    # Koniec pracy.
    with self.work.mutex:  # Zajęcie muteksu w stylu RAII.
      self.work.work_complete = True
      self.work.cond.notifyAll()

class ConsumerThread(Thread):
  def __init__(self, work):
    super(ConsumerThread, self).__init__()
    self.work = work

  def run(self):
    self.work.mutex.acquire()
    while True:

      # Sprawdź, czy jest do wykonania jakokolwiek praca oraz czy zakończyliśmy
      # pracę.
      data = None
      if self.work.work:
        data = self.work.work.pop(0)
      elif self.work.work_complete:
        self.work.mutex.release()
        print "Work complete!"
        break  # Koniec

      # W przypadku braku danych, przejdź do oczekiwania na dane.
      if data is None:
        self.work.cond.wait()
        continue

      # Przetworzenie danych - muteks jest niepotrzebny. Jeśli w międzyczasie
      # nowe dane zostaną dodane, to obecny wątek nie otrzyma powiadomienia
      # (ponieważ nie jest w stanie oczekiwania), dlatego istotne jest by w
      # kolejnej iteracji pętli ponownie sprawdzić czy są dane.
      self.work.mutex.release()

      print "Working on data: %i" % data
      sleep(0.1)  # Ciężka praca. Dużo ciężkiej pracy.
      print "Done working on: %i" % data

      # Zajmij ponownie muteks.
      self.work.mutex.acquire()

def main():
  # Start the threads.
  work = WorkContainer()
  pro_th = ProducerThread(work)
  con_th = ConsumerThread(work)
  pro_th.start()
  con_th.start()
  print "Threads started"

  # Wait until they are done.
  pro_th.join()
  con_th.join()

if __name__ == "__main__":
  main()



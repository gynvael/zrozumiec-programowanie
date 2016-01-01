#!/usr/bin/python
# -*- coding: utf-8 -*-
from threading import Thread
import Queue
import sys

done = False

class MyProducer(Thread):
  def __init__(self, q):
    super(MyProducer, self).__init__()
    self.q = q

  def run(self):
    global done

    # Umieść 100 zadań w kolejce.
    for i in xrange(10):
      self.q.put(i)

    # Poczekaj aż wszystkie zostaną wybrane i przetworzone.
    self.q.join()
    done = True
    
    # Poniżej używam stdout.write zamiast print, ponieważ print nie jest
    # operacją atomową - znak nowej linii jest wypisywany w oddzielnej operacji
    # przez co w środowisku wielowątkowym dochodzi do sytuacji wyścigu, gdzie
    # kilka wiadomości może trafić do tej samej linii, co mniejsza czytelność.
    # W przypadku użycia sys.stdout.write dla krótkiej wiadomości istnieje
    # znaczne prawdopodobieństwo, że cała wiadomość zostanie przepisana do
    # buforu stdout w jednej operacji.
    sys.stdout.write("[P] Done.\n")


class MyConsumer(Thread):
  def __init__(self, q, tid):
    super(MyConsumer, self).__init__()
    self.q = q
    self.tid = tid

  def run(self):
    global done
    while not done:
      try:
        work = self.q.get(timeout=0.01)
        sys.stdout.write("[%i] %u\n" % (self.tid, work))
        self.q.task_done()
      except Queue.Empty:
        pass  # Brak pracy w kolejce.

    sys.stdout.write("[%i] Exiting.\n" % self.tid)


def main():
  # Uruchom wątki.
  q = Queue.Queue()
  threads = [ MyProducer(q) ]
  threads.extend([ MyConsumer(q, i) for i in xrange(8) ])

  # Uruchom wątki.
  for th in threads: th.start()

  # Poczekaj aż się zakończą.
  for th in threads: th.join()


if __name__ == "__main__":
  main()


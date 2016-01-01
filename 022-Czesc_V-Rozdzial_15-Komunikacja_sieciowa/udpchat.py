#!/usr/bin/python
# -*- coding: utf-8 -*-
# Uwaga: poniższy, przykładowy kod nie ma pełnego sprawdzania błędów.
import sys
import os
import time
import socket
import json
import hashlib
from threading import Event, Lock, Thread
from struct import pack, unpack

# Domyślny port - można go zmienić podając inny w argumencie.
CHAT_PORT = 59999

PY3 = False
if sys.version_info.major == 3:
  PY3 = True


# Wątek odbierający wiadomości.
class Receiver(Thread):
  def __init__(self, s, the_end, p2pchat):
    super(Receiver, self).__init__()
    self.s = s
    self.the_end = the_end
    self.p2pchat = p2pchat

  def run(self):
    while not self.the_end.is_set():
      try:
        packet, addr = self.s.recvfrom(0xffff)  # Maksymalna wielkość pakietu UDP/IPv4.
        if PY3:
          packet = str(packet, 'utf-8')
        packet = json.loads(packet)
        t = packet["type"]
      except socket.timeout as e:
        continue
      except ValueError as e:
        # Przypadek, gdy dane nie są prawidłowym JSONem.
        continue
      except KeyError as e:
        # Przypadek, gdy packet nie ma zdefiniowanego klucza „type”.
        continue
      except TypeError as e:
        # Przypadek, gdy packet nie jest słownikiem.
        continue
      addr = "%s:%u" % addr
      self.p2pchat.handle_incoming(t, packet, addr)
    self.s.close()


class P2PChat():
  def __init__(self):
    self.nickname = ''
    self.s = None
    self.the_end = Event()
    self.nearby_users = set()
    self.known_messages = set()
    self.id_counter = 0
    self.unique_tag = os.urandom(16)

  def main(self):
    # Alternatywnie w Python 3 możnaby napisać:
    # print("Enter your nickname: ", end="", flush=True)
    sys.stdout.write("Enter your nickname: ")
    sys.stdout.flush()
    nickname = sys.stdin.readline()
    if not nickname:
      return
    self.nickname = nickname.strip()

    # Przetwórz początkowe IP innych użytkowników.
    port = CHAT_PORT
    if len(sys.argv) == 2:
      port = int(sys.argv[1])

    print("Creating UDP socket at port %u.\n"
          "To change the port, restart the app like this: udpchat.py <port>\n" %
          port)

    # Stwórz gniazdo UDP na porcie 59999 i wszystkich interfejsach.
    self.s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    self.s.settimeout(0.2)
    self.s.bind(("0.0.0.0", port))
  
    # Uruchom wątek odbierający dane.
    th = Receiver(self.s, self.the_end, self)
    th.start()

    print("To start, please add another user's address, e.g.:\n"
          "    /add 1.2.3.4\n"
          "    /add 1.2.3.4:59999\n"
          "    /add gynvael.coldwind.pl:45454\n"
          "Or: wait for a message from someone else.\n")

    # Przejdź do głównej pętli.
    try:
      while not self.the_end.is_set():
        sys.stdout.write("? ")
        sys.stdout.flush()

        # Odczytaj linię od użytkownika.
        ln = sys.stdin.readline()
        if not ln:
          self.the_end.set()
          continue

        ln = ln.strip()
        if not ln:
          continue

        if ln[0] == '/':
          # Polecenie.
          cmd = [l for l in ln.split(' ') if len(l) > 0]
          self.handle_cmd(cmd[0], cmd[1:])
        else:
          # Wiadomość.
          self.send_message(ln)
    except KeyboardInterrupt as e:
      self.the_end.set()

    # Receiver powinien zamknąć gniazdo jak zakończy działanie.
    print("Bye!")

  def handle_incoming(self, t, packet, addr):
    # Pakiet z informacją o nowym sąsiedującym węźle w sieci P2P.
    if t == "HELLO":
      print("# %s/%s connected" % (addr, packet["name"]))
      self.add_nearby_user(addr)
      return

    # Pakiet z wiadomością tekstową.
    if t == "MESSAGE":
      # Jeśli nadawca był do tej pory nieznany, dodaj go do setu sąsiadujących
      # węzłów.
      self.add_nearby_user(addr)

      # Sprawdź, czy tej wiadomości czasem nie dostaliśmy od innego węzła z
      # sieci.
      if packet["id"] in self.known_messages:
        return
      self.known_messages.add(packet["id"])

      # Dopisz nadawcę wiadomości do listy węzłów przez które przeszła
      # wiadomość.
      packet["peers"].append(addr)

      # Wyświetl wiadomość i jej trasę.
      print("\n[sent by: %s]" % ' --> '.join(packet["peers"]))
      print("<%s> %s" % (packet["name"], packet["text"]))

      # Wyślij wiadomośc do sąsiadujących węzłów.
      self.send_packet(packet, None, addr)
  
  def handle_cmd(self, cmd, args):
    # W przypadku /quit, zakończ program.
    if cmd == "/quit":
      self.the_end.set()
      return

    # W przypadku manualnego dodawania węzłów, upewnij się że są poprawnie
    # zapisane, przetłumacz domenę (DNS) na adres IP, i dopisz do setu
    # sąsiadujących węzłów.
    if cmd == "/add":
      for p in args:
        port = CHAT_PORT
        addr = p
        try:        
          if ':' in p:
            addr, port = p.split(':', 1)
            port = int(port)
          addr = socket.gethostbyname(addr)
        except ValueError as e:
          print("# address %s invalid (format)" % p)
          continue
        except socket.gaierror as e:
          print("# host %s not found" % addr)
          continue
        addr = "%s:%u" % (addr, port)
        self.add_nearby_user(addr)
      return

    # Nieznane polecenie.
    print("# unknown command %s" % cmd)

  def add_nearby_user(self, addr):
    # Sprawdź, czy węzeł nie jest czasem już znany.
    if addr in self.nearby_users:
      return

    # Dodaj węzeł i wyślij mu wiadomość powitalną.
    self.nearby_users.add(addr)
    self.send_packet({
      "type": "HELLO",
      "name": self.nickname
      }, addr)

  def send_message(self, msg):
    # Wylicz unikatowy identyfikator wiadomości.
    hbase = "%s\0%s\0%u\0" % (self.nickname, msg, self.id_counter)
    self.id_counter += 1
    if PY3:
        hbase = bytes(hbase, 'utf-8')
    h = hashlib.md5(hbase + self.unique_tag).hexdigest()

    # Wyślij pakiet z wiadomością do wszystkich znanych węzłów.
    self.send_packet({
      "type": "MESSAGE",
      "name": self.nickname,
      "text": msg,
      "id": h,
      "peers": []
      })

  def send_packet(self, packet, target=None, exclude=set()):
    # Zserializuj pakiet.
    packet = json.dumps(packet)
    if PY3:
      packet = bytes(packet, 'utf-8')

    # Jeśli nie ma podanego żadnego docelowego węzła, należy wysłać wiadomość
    # do wszystkich węzłów oprócz tych z setu exclude.
    if not target:
      target = list(self.nearby_users)
    else:
      target = [target]

    for t in target:
      if t in exclude:
        continue

      # Zakładam, że w tym momencie wszystkie addresy są poprawnie sformatowane.
      addr, port = t.split(":")
      port = int(port)

      # Faktyczne wysłanie pakietu.
      self.s.sendto(packet, (addr, port))

    
def main():
  p2p = P2PChat()
  p2p.main()

  
if __name__ == "__main__":
  main()

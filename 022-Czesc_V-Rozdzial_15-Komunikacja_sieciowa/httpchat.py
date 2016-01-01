#!/usr/bin/python
# -*- coding: utf-8 -*-
# Uwaga: Ponieważ w Python 3 metoda socket.recv zwraca typ bytes a nie str, a
# większość kodu wyższego poziomu operuje na stringach (które nie są w pełni
# kompatybilne z klasą bytes, w szczególności jeśli chodzi o porównania lub
# użycie jako klucz w słowniku), w kilku miejscach program sprawdza z którą
# wersją interpretera ma do czynienia i wybiera odpowiednią wersję kodu do
# wykonania.
import os
import json
import sys
import socket
from threading import Event, Lock, Thread

DEBUG = False  # Zmiana na True powdouje wyświetlenie dodatkowych komunikatów.

# Implementacja logiki strony WWW.
class SimpleChatWWW():
  def __init__(self, the_end):
    self.the_end = the_end
    self.files = "."  # Na potrzeby przykładu pliki mogą być w katalogu
                      # roboczym.

    self.file_cache = {}
    self.file_cache_lock = Lock()

    self.messages = []
    self.messages_offset = 0
    self.messages_lock = Lock()
    self.messages_limit = 1000  # Jak długa historia ma być przechowywana.

    # Mapowanie adresów WWW do metod obsługujących.
    self.handlers = {
        ('GET', '/'): self.__handle_GET_index,
        ('GET', '/index.html'): self.__handle_GET_index,
        ('GET', '/style.css'): self.__handle_GET_style,
        ('GET', '/main.js'): self.__handle_GET_javascript,
        ('POST', '/chat'): self.__handle_POST_chat,
        ('POST', '/messages'): self.__handle_POST_messages,
        }

  def handle_http_request(self, req):
    req_query = (req['method'], req['query'])
    if req_query not in self.handlers:
      return { 'status': (404, 'Not Found') }
    return self.handlers[req_query](req)

  def __handle_GET_index(self, req):
    return self.__send_file('httpchat_index.html')

  def __handle_GET_style(self, req):
    return self.__send_file('httpchat_style.css')

  def __handle_GET_javascript(self, req):
    return self.__send_file('httpchat_main.js')

  def __handle_POST_chat(self, req):
    # Odczytaj potrzebne pola z otrzymanego obiektu JSON. Bezpiecznie jest nie
    # czynić żadnych założeń co do zawartości i typu przesyłanych danych.
    try:
      obj = json.loads(req['data'])
    except ValueError:
      return { 'status': (400, 'Bad Request') }

    if type(obj) is not dict or 'text' not in obj:
      return { 'status': (400, 'Bad Request') }

    text = obj['text']
    if type(text) is not str and type(text) is not unicode:
      return { 'status': (400, 'Bad Request') }

    sender_ip = req['client_ip']

    # Dodaj wiadomość do listy. Jeśli lista jest dłuższa niż limit, usuń jedną
    # wiadomość z przodu i zwiększ offset.
    with self.messages_lock:
      if len(self.messages) > self.messages_limit:
        self.messages.pop(0)
        self.messages_offset += 1
      self.messages.append((sender_ip, text))

    sys.stdout.write("[  INFO ] <%s> %s\n" % (sender_ip, text))

    return { 'status': (200, 'OK') }
 
  def __handle_POST_messages(self, req):
    # Odczytaj potrzebne pola z otrzymanego obiektu JSON. Bezpiecznie jest nie
    # czynić żadnych założeń co do zawartości i typu przesyłanych danych.
    try:
      obj = json.loads(req['data'])
    except ValueError:
      return { 'status': (400, 'Bad Request') }

    if type(obj) is not dict or 'last_message_id' not in obj:
      return { 'status': (400, 'Bad Request') }

    last_message_id = obj['last_message_id']
    if type(last_message_id) is not int:
      return { 'status': (400, 'Bad Request') }

    # Skopiuj wiadomości od last_message_id.
    with self.messages_lock:
      last_message_id -= self.messages_offset
      if last_message_id < 0:
        last_message_id = 0
      messages = self.messages[last_message_id:]
      new_last_message_id = self.messages_offset + len(self.messages)

    # Wyślij odpowiedź.
    data = json.dumps({
      "last_message_id": new_last_message_id,
      "messages": messages
      })
    
    return { 
        'status': (200, 'OK'),
        'headers': [
            ('Content-Type', 'application/json;charset=utf-8'),
            ],
        'data': data
        }

  # Stworzenie odpowiedzi z danymi z pliku obecnego na dysku. W praktyce
  # poniższa metoda dodatkowo stara się przechowywać pliki w pamięci
  # podręcznej i wysyłać je tylko jeśli ich wcześniej nie wczytała lub jeśli
  # plik się zmienił w międzyczasie.
  def __send_file(self, fname):
    # Ustal typ pliku po rozszerzeniu.
    ext = os.path.splitext(fname)[1]
    mime_type = {
        '.html': 'text/html;charset=utf-8',
        '.js': 'application/javascript;charset=utf-8',
        '.css': 'text/css;charset=utf-8',
        }.get(ext.lower(), 'application/octet-stream')

    # Sprawdź, kiedy plikzostał ostatnio zmodyfikowany.
    try:
      mtime = os.stat(fname).st_mtime
    except:
      # Niestety, CPython na Windows rzuca klasę wyjątków, która nie jest
      # zadeklarowana pod GNU/Linux. Najprościej jest złapać wszystkie
      # wyjątki, choć jest to zdecydowanie nieeleganckie.
      # Plik prawdopodobnie nie istnieje lub nie ma do niego dostępu.
      return { 'status': (404, 'Not Found') }

    # Sprawdź czy plik jest w pamięci podręcznej.      
    with self.file_cache_lock:
      if fname in self.file_cache and self.file_cache[fname][0] == mtime:
        return {
            'status': (200, 'OK'),
            'headers': [
                ('Content-Type', mime_type),
                ],
            'data': self.file_cache[fname][1]
            }

    # W ostateczności wczytaj plik.
    try:
      with open(fname, 'rb') as f:
        data = f.read()
        mtime = os.fstat(f.fileno()).st_mtime  # Uaktualnij mtime.
    except IOError as e:
      # Nie udało się odczytać pliku.
      if DEBUG:
        sys.stdout.write("[WARNING] File %s not found, but requested.\n" % fname)
      return { 'status': (404, 'Not Found') }

    # Dodaj zawartość pliku do pamięci podręcznej (chyba, że inny wątek zrobił
    # to w międzyczasie).
    with self.file_cache_lock:
      if fname not in self.file_cache or self.file_cache[fname][0] < mtime:
        self.file_cache[fname] = (mtime, data)

    # Odeślij odpowiedź z danymi pliku.
    return {
        'status': (200, 'OK'),
          'headers': [
            ('Content-Type', mime_type),
            ],
          'data': data
          }
 

# Bardzo prosta implementacja wielowątkowego serwera HTTP.
class ClientThread(Thread):
  def __init__(self, website, sock, sock_addr):
    super(ClientThread, self).__init__()    
    self.s = sock
    self.s_addr = sock_addr
    self.website = website

  def __recv_http_request(self):
    # Bardzo uproszczony parsing zapytania HTTP, którego głównym celem jest
    # wydobycie:
    # - metody
    # - żądanej ścieżki
    # - kolejnych parametrów w formie słownika
    # - dodatkowych danych (w przypadku POST)

    # Odbierz dane aż do zakończenia nagłówka.
    data = recvuntil(self.s, '\r\n\r\n')
    if not data:
      return None

    # Podziel na linie.
    lines = data.split('\r\n')

    # Przeanalizuj zapytanie (pierwsza linia).
    query_tokens = lines.pop(0).split(' ')
    if len(query_tokens) != 3:
      return None

    method, query, version = query_tokens

    # Wczytaj parametry.
    headers = {}
    for line in lines:
      tokens = line.split(':', 1)
      if len(tokens) != 2:
        continue
      # Nazwa nagłówka jest case-insensitive, więc warto ją znormalizować, np.
      # zmieniając wszystkie litery na małe.
      header_name = tokens[0].strip().lower()
      header_value = tokens[1].strip()
      headers[header_name] = header_value

    # W przypadku metody POST pobierz dodatkowe dane.
    # Uwaga: przykładowa implementacja w żaden sposób nie limituje ilości
    # przesyłanych danych.
    if method == 'POST':
      try:
        data_length = int(headers['content-length'])
        data = recv_all(self.s, data_length)
      except KeyError as e:
        # Brak Content-Length w nagłówkach.
        data = recv_remaining(self.s)
      except ValueError as e:
        return None      
    else:
      data = None

    # Umieść wszystkie istotne dane w słowniku i go zwróć.
    request = {
        "method": method,
        "query": query,
        "headers": headers,
        "data": data,
        "client_ip": self.s_addr[0],
        "client_port": self.s_addr[1]
        }

    return request

  def __send_http_response(self, response):
    # Skonstruuj odpowiedź HTTP.
    lines = []
    lines.append('HTTP/1.1 %u %s' % response['status'])

    # Ustaw podstawowe pola.
    lines.append('Server: example')
    if 'data' in response:
      lines.append('Content-Length: %u' % len(response['data']))
    else:
      lines.append('Content-Length: 0')

    # Przepisz nagłówki.
    if 'headers' in response:
      for header in response['headers']:
        lines.append('%s: %s' % header)
    lines.append('')

    # Przepisz dane.
    if 'data' in response:
      lines.append(response['data'])
    
    # Ew. skonwertuj odpowiedź na bajty i wyślij.
    if sys.version_info.major == 3:    
      converted_lines = []
      for line in lines:
        if type(line) is bytes:
          converted_lines.append(line)
        else:
          converted_lines.append(bytes(line, 'utf-8'))
      lines = converted_lines
    
    self.s.sendall(b'\r\n'.join(lines))

  def __handle_client(self):
    request = self.__recv_http_request()
    if not request:
      if DEBUG:
        sys.stdout.write("[WARNING] Client %s:%i doesn't make any sense. "
                         "Disconnecting.\n" % self.s_addr)
      return

    if DEBUG:
      sys.stdout.write("[  INFO ] Client %s:%i requested %s\n" % (
          self.s_addr[0], self.s_addr[1], request['query']))
    response = self.website.handle_http_request(request)
    self.__send_http_response(response)

  def run(self):
    self.s.settimeout(5)  # Operacje nie powinny zajmować dłużej niż 5 sekund.
    try:
      self.__handle_client()
    except socket.timeout as e:
      if DEBUG:
        sys.stdout.write("[WARNING] Client %s:%i timed out. "
                         "Disconnecting.\n" % self.s_addr)
    self.s.shutdown(socket.SHUT_RDWR)
    self.s.close()


# Niezbyt szybka, ale wygodna funkcja odbierająca dane aż do napotkania
# konkretnego ciągu znaków (który również jest zwracany).
def recvuntil(sock, txt):
  txt = list(txt)
  if sys.version_info.major == 3:
    txt = [bytes(ch, 'ascii') for ch in txt]

  full_data = []
  last_n_bytes = [None] * len(txt)

  # Do póki ostatnie N bajtów nie będzie równe poszukiwanym, odczytuj dane.
  while last_n_bytes != txt:
    next_byte = sock.recv(1)
    if not next_byte:
      return ''  # Połączenie zostało zerwane.
    full_data.append(next_byte)
    last_n_bytes.pop(0)
    last_n_bytes.append(next_byte)

  full_data = b''.join(full_data)
  if sys.version_info.major == 3:
    return str(full_data, 'utf-8')
  return full_data


# Pomocnicza funkcja odbierająca dokładnie określoną liczbę bajtów.
def recv_all(sock, n):
  data = []
  data_len = 0
  while data_len != n:
    data_latest = sock.recv(n - data_len)
    if not data_latest:
      return None
    data.append(data_latest)
    data_len += len(data_latest)

  data = b''.join(data)
  if sys.version_info.major == 3:
    return str(data, 'utf-8')
  return data

# Pomocnicza funkcja odbierająca dane aż do rozłączenia.
def recv_remaining(sock):
  data = []
  while True:
    data_latest = sock.recv(4096)
    if not data_latest:
      data = b''.join(data)
      if sys.version_info.major == 3:
        return str(data, 'utf-8')
      return data      
    data.append(data_latest)


def main():
  the_end = Event()
  website = SimpleChatWWW(the_end)

  # Stwórz gniazdo nasłuchujące na porcie 8888 na wszystkich interfejsach.
  s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

  # W przypadku GNU/Linux należy wskazać, iż ten sam adres lokalny powinien być
  # możliwy do wykorzystania od razu po zamknięciu gniazda. W innym wypadku
  # adres będzie w stani„e kwarantanny” (TIME_WAIT) przez 60 sekund i w tym czasie
  # ponowna próba powiązania z nim gniazda zakończy się błędem.
  s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)    

  s.bind(('0.0.0.0', 8888))
  s.listen(32) # Liczba w nawiasie mówi o maksymalnej wielkości kolejki
               # oczekujących połączeń. W tym wypadku połączenia będą odbierane
               # na bieżąco, wiec kolejka może być niewielka. Typowe wartości
               # to 128 (GNU/Linux) lub „kilkaset” (Windows).

  # Ustaw timeout na gnieździe, tak by blokujące operacje wykonywane na nim
  # były przerywane co sekundę (dzięki temu kod będzie mógł sprawdzić, serwer
  # został wezwany do zakończenia pracy).
  s.settimeout(1)

  while not the_end.is_set():
    # Odbierz połączenie.
    try:
      c, c_addr = s.accept()
      c.setblocking(1)  # W niektórych implementacjach języka Python gniazdo
                        # zwrócone przez accept na gnieździe nasłuchującym z
                        # ustawionym timeoutem jest domyślnie asychroniczne,
                        # co jest niepożądanym zachowaniem.
      if DEBUG:
        sys.stdout.write("[  INFO ] New connection: %s:%i\n" % c_addr)
    except socket.timeout as e:
      continue  # Sprawdź warunek końca.

    # Nowe połączenie. Stwórz nowy wątek do jego obsługi (alternatywnie
    # można by w tym miejscu skorzystać z puli wątków - threadpool).
    ct = ClientThread(website, c, c_addr)
    ct.start()


if __name__ == "__main__":
  main()


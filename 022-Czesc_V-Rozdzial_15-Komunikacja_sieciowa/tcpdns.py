#!/usr/bin/python
# -*- coding: utf-8 -*-
import os
import sys
import socket
from struct import pack, unpack
from datetime import timedelta

def dns_query(query, domain, dnserver):
  # Stwórz gniazdo korzystające z TCP (AF_INET, SOCK_STREAM).
  s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

  # Połącz się ze wskazanym serwerem na porcie 53.
  try:
    s.connect((dnserver, 53))  # DNS działa na porcie 53.
  except socket.error as e:
    sys.stderr.write("error: failed to connect to server (%s)\n" % e.strerror)
    return None

  # Wyślij zapytanie.
  dns_query_packet = dns_query_make_packet(query, domain)
  dns_tcp_send_packet(s, dns_query_packet)

  # Odbierz odpowiedź.
  dns_response_packet = dns_tcp_recv_packet(s)
  dns_reply = dns_response_parse_packet(dns_response_packet)

  # Zamknij połączenie i gniazdo.
  s.shutdown(socket.SHUT_RDWR)
  s.close()
  
  return dns_reply

def dns_query_make_packet(query_type, domain):
  # Stworzenie nagłówka.
  query_id = 1234  # W przypadku pojedynczej aplikacji i TCP pole to może mieć
                   # dowolną wartość (jest w zasadzie nieużywane).

  # W podstawowym użyciu większość pól z 16-bitowego słowa sterującego może być
  # wyzerowana.
  cw_rcode = 0
  cw_z = 0
  cw_ra = 0
  cw_rd = 1  # Rozwiąż zapytanie rekursywnie.
  cw_tc = 0
  cw_aa = 0
  cw_opcode = 0  # Standardowe zapytanie (QUERY).
  cw_qr = 0  # Zapytanie.

  # Złożenie pojedynczych pól do 16-bitowego słowa.
  control_word = (
      cw_rcode |
      (cw_z << 4) |
      (cw_ra << 7) |
      (cw_rd << 8) |
      (cw_tc << 7) |
      (cw_aa << 10) |
      (cw_opcode << 11) |
      (cw_qr << 15))

  header = pack(">HHHHHH", 
      query_id,  # ID
      control_word,  # QR, Opcode, AA, TC, RD, RA, Z, RCODE
      1,  # QDCOUNT
      0,  # ANCOUNT
      0,  # NSCOUNT 
      0)  # ARCOUNT

  # Stworzenie zapytania.
  query = []

  # Zakodowanie poszczególnych elementów domeny w formie długość+dane.
  for subdomain in domain.split("."):
    query.append(pack(">B", len(subdomain)))
    query.append(subdomain)
  query.append("\0")  # Ostatni element ma długość zero i oznacza koniec.
  
  qtype = {
      "A": 1,
      "NS": 2,
      "CNAME": 5,
      "SOA": 6,
      "WKS": 11,
      "PTR": 12,
      "HINFO": 13,
      "MINFO": 14,
      "MX": 15,
      "TXT": 16      
      }[query_type]
  qclass = 1  # IN (the Internet).
  query.append(pack(">HH", qtype, qclass))

  return header + ''.join(query)

def dns_response_parse_packet(p):
  idx = 0
  header = dns_response_parse_header(p)
  idx += 12

  # Zignoruj powtórzone zapytania - w naszym przypadku są niepotrzebne.
  for _ in range(header["QDCOUNT"]):
    domain, idx = dns_decode_domain(p, idx)
    idx += 4  # Zignoruj pola TYPE i CLASS.

  # Odbierz odpowiedzi.
  reply = []
  for _ in range(header["ANCOUNT"]):
    domain, idx = dns_decode_domain(p, idx)
    atype, aclass, attl, adatalen = unpack(">HHIH", p[idx:idx+10])
    idx += 10
    adata = p[idx:idx+adatalen]
    idx += adatalen

    reply.append({
      "TYPE": dns_type_to_str(atype),
      "CLASS": dns_class_to_str(aclass),
      "TTL": dns_ttl_to_str(attl),
      "DATA": dns_data_to_str(atype, adata, p, idx - adatalen)
      })

  return reply

def dns_response_parse_header(p):
  query_id, control_word, qdcount, ancount, nscount, arcount = (
    unpack(">HHHHHH", p[:12]))
  header = {
      "QDCOUNT": qdcount,
      "ANCOUNT": ancount
      }
  return header

def dns_class_to_str(aclass):
  return {
      1: "IN", 2: "CS", 3: "CH", 4: "HS"
      }.get(aclass, "??")

def dns_type_to_str(atype):
  return {
      1: "A", 2: "NS", 5: "CNAME", 6: "SOA", 11: "WKS", 12: "PTR", 13: "HINFO",
      14: "MINFO", 15: "MX", 16: "TXT"
      }.get(atype, "??")

def dns_ttl_to_str(attl):
  return str(timedelta(seconds=attl))

def dns_data_to_str(atype, adata, p, adata_idx):
  if atype == 1:  # A record.
    return "%u.%u.%u.%u" % unpack("BBBB", adata)
  elif atype == 15:  # MX record.
    preference = unpack(">H", adata[:2])[0]
    domain, _ = dns_decode_domain(p, adata_idx + 2)
    return "%s (%u)" % (domain, preference)

  # W przypadku nieobsługiwanego typu, wypisz drukowalne znaki. Znaki (bajty)
  # niedrukowalne wg. ASCII zastąp heksadecymalnym zapisem ich kodu.
  o = []
  for ch in adata:
    ch = ord(ch)
    if 32 <= ch <= 127:  # Python jest jednym z niewielu języków, które
                         # umożliwiają zapis tego typu.
      o.append(chr(ch))
    else:
      o.append(" [%.2x] " % ch)

  return ''.join(o)

# Rekursywny (z uwagi na kompresje) odczyt nazwy domeny.
def dns_decode_domain(p, idx):
  domain = []

  while True:
    type_len = ord(p[idx])
    idx += 1

    if type_len == 0:  # Koniec.
      break

    if type_len & 0xc0:  # Kompresja (wskaźnik)
      # Zdekoduj offset nazwy.
      offset = (type_len & 0x3f) << 8
      offset |= ord(p[idx])
      idx += 1

      # Pobierz nazwę domeny ze wskazanego offsetu.
      domain_part, _ = dns_decode_domain(p, offset)
      domain.append(domain_part)

      # Wskaźnik jest zawsze ostatnim elementem domeny.
      break

    # Kolejny, zwykły fragment domeny.
    domain_part = p[idx:idx+type_len]
    domain.append(domain_part)
    idx += type_len

  return '.'.join(domain), idx

# DNS wymaga by każdy pakiet był poprzedzony 2-bajtowym polem określającym jego
# długość.
def dns_tcp_send_packet(s, packet):
  packet_len = pack(">H", len(packet))
  s.sendall(packet_len)
  s.sendall(packet)

def dns_tcp_recv_packet(s):
  packet_len = recv_all(s, 2)
  if not packet_len:
    return None
  packet_len = unpack(">H", packet_len)[0]
  return recv_all(s, packet_len)

# Pomocnicza funkcja odbierająca dokładnie określoną liczbę bajtów.
def recv_all(s, n):
  d = []
  d_len = 0
  while d_len != n:
    d_latest = s.recv(n - d_len)
    if len(d_latest) == 0:
      # Druga strona rozłączyła się przed przesłaniem wszystkich wymaganych
      # danych.
      return None
    d.append(d_latest)
    d_len += len(d_latest)
  return ''.join(d)

print dns_query("A", "gynvael.coldwind.pl", "8.8.8.8")
print dns_query("MX", "coldwind.pl", "8.8.8.8")
print dns_query("TXT", "coldwind.pl", "dns1.domeny.tv")


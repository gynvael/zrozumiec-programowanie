#!/usr/bin/python
# -*- coding: utf-8 -*-
import os
import stat
import sys
import pwd
import grp

def uid_to_username(uid):
  try:
    return pwd.getpwuid(uid).pw_name
  except KeyError as e:
    return "???"

def gid_to_groupname(gid):
  try:
    return grp.getgrgid(gid).gr_name
  except KeyError as e:
    return "???"

def mod_to_string(mod):
  perms = ""
  perms += "-r"[bool(mod & 4)]
  perms += "-w"[bool(mod & 2)]
  perms += "-x"[bool(mod & 1)]
  return perms

def special_to_string(mod):
  spec = []
  if mod & 0o4000: spec.append("SUID")
  if mod & 0o2000: spec.append("SGID")
  if mod & 0o1000: spec.append("sticky")  
  return ' '.join(spec)

def main():
  for fname in sys.argv[1:]:
    print("%s %s" % ("-" * 40, fname))
    try:
      s = os.stat(fname)
      user_name = uid_to_username(s.st_uid)
      group_name = gid_to_groupname(s.st_gid)
      perms_owner = mod_to_string(s.st_mode >> 6)
      perms_group = mod_to_string(s.st_mode >> 3)
      perms_others = mod_to_string(s.st_mode)
      special = special_to_string(s.st_mode)
      print("%-14s: %s" % ("Owner", user_name))
      print("%-14s: %s" % ("Group", group_name))
      print("%-14s: %s" % ("Perms (owner)", perms_owner))
      print("%-14s: %s" % ("Perms (group)", perms_group))
      print("%-14s: %s" % ("Perms (others)", perms_others))
      print("%-14s: %s" % ("Special", special))
    except OSError as e:
      print("error accessing file: %s" % e.strerror)

if __name__ == "__main__":
  main()


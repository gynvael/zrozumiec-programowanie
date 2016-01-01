#!/usr/bin/python
# -*- coding: utf-8 -*-
import posix1e
import pwd
import grp

TAGS_WITH_QUALIFIER = [ posix1e.ACL_USER, posix1e.ACL_GROUP ]

def print_acl_info(fn):
  acl = posix1e.ACL(file=fn)
  for entry in acl:
    if entry.tag_type in TAGS_WITH_QUALIFIER:
      # Wpis dla konkretnego użytkownika lub grupy.
      user_name = "???"
      try:
        # Mając ID użytkownika lub grupy, znajdź odpowiadającą nazwę w
        # /etc/passwd lub /etc/group.
        if entry.tag_type == posix1e.ACL_USER:
          user_name = pwd.getpwuid(entry.qualifier).pw_name
        else:
          user_name = grp.getgrgid(entry.qualifier).gr_name
      except KeyError as e:
        # Brak wpisu.
        pass
      print("%-40s: %s (%s)" % (entry, entry.permset, user_name))
    else:
      # Wpis ogólny.
      print("%-40s: %s" % (entry, entry.permset))

def _remove_dup(acl, entry):
  # Znajdź i usuń wszystkie duplikaty, tj.:
  # - W przypadku ogólnych wpisów (user/group/other) usuń wskazany ogólny wpis,
  #   tj. taki, w którym zgadza się tag_type.
  # - W przypadku konkretnych wpisów (user obj/group obj) usuń wskazany 
  #   konkretny wpis, tj. taki w którym zgadza się zarówno tag_type, jak i
  #   qualifier.
  for e in acl:
    if e.tag_type == entry.tag_type:
      if e.tag_type in TAGS_WITH_QUALIFIER and e.qualifier == entry.qualifier:
        acl.delete_entry(e)
      else:
        acl.delete_entry(e)

def _reset_acl(fn, s, append):
  file_acl = posix1e.ACL(file=fn)
  acl = posix1e.ACL(text=s)

  for entry in acl:
    _remove_dup(file_acl, entry)
    if append:
      file_acl.append(entry)

  file_acl.calc_mask()  # Przelicz maskę.

  if not file_acl.valid():
    print("invalid ACL provided: %s" % acl)
    return False

  # Zaaplikuj przeliczone ACL.
  file_acl.applyto(fn)
  return True  

def remove_acl(fn, s):
  return _reset_acl(fn, s, False)

def add_acl(fn, s):
  return _reset_acl(fn, s, True)

# Kilka przykładowych testów powyższych funkcji.
test_file = "/home/gynvael/testfile"  # Plik musi istnieć.
test_user = "www-data"

print("---- Original ACL:")
print_acl_info(test_file)

print("---- Adding user %s to ACL with RWX:" % test_user)
add_acl(test_file, "u:%s:rwx" % test_user)
print_acl_info(test_file)

print("---- Changin %s's access to R-X:" % test_user)
add_acl(test_file, "u:%s:r-x" % test_user)
print_acl_info(test_file)

print("---- Removing %s from ACLs:" % test_user)
remove_acl(test_file, "u:%s:-" % test_user)
print_acl_info(test_file)


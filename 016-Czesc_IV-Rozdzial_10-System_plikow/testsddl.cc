#include <windows.h>
#include <sddl.h>

#include <iostream>
#include <sstream>
#include <string>


int main() {
  const std::string file_path("e:\\shared\\SharedFile.txt");
  // Katalog e:\\shared posiada następujące wpisy na liście kontroli dostępu:
  //   windows\gynvael:(I)(OI)(CI)(F)
  //   windows\anotheruser:(RX)
  // Domyślnie utworzenie nowego pliku w tym katalogu nie pozwala użytkownikowi
  // anotheruser na jakikolwiek dostęp do pliku.

  // Podobnie jak na GNU/Linux, należy przetłumaczyć nazwę użytkownika na jego
  // identyfikator (SID).
  BYTE sid_bytes[SECURITY_MAX_SID_SIZE];
  DWORD sid_size = sizeof(sid_bytes);
  CHAR domain_name[256];
  DWORD domain_name_size = sizeof(domain_name);
  SID_NAME_USE sid_type;

  BOOL ret = LookupAccountName(
      NULL, "media",
      sid_bytes, &sid_size,
      domain_name, &domain_name_size,
      &sid_type);

  if (!ret) {
    DWORD last_error = GetLastError();
    std::cerr << "failed to find desired user (" << last_error << ")" 
              << std::endl;
    return 1;
  }

  // SID jest wymagany w formie tekstowej - należy dokonać konwersji. Funkcja
  // ConvertSidToStringSid alokuje bufor na tekst za pomocą LocalAlloc - należy
  // go zwolnić po zakończeniu użycia przy użyciu LocalFree.
  char *another_user_sid;
  ConvertSidToStringSid(sid_bytes, &another_user_sid);

  // Można złożyć 
  std::stringstream dacl;
  dacl << "D:"  // DACL.
       // Allow, Generic All (pełen zestaw praw), Creator / Owner.
       << "(A;;GA;;;CO)"
       // Allow, Generic Read (tylko-do-odczytu), anotheruser.
       << "(A;;GR;;;" << another_user_sid << ")";

  LocalFree(another_user_sid);
  std::cout << "using permissions: " << dacl.str() << "\n";

  // Teraz pozostaje dokonać konwersji formy tekstowej na zestaw struktur
  // przyjmowany przez CreateFile.  
  SECURITY_ATTRIBUTES sa;
  sa.nLength = sizeof(SECURITY_ATTRIBUTES);
  sa.bInheritHandle = FALSE;

  // Nagroda za najdłuższą nazwę funkcji w niniejszej książce idzie do:
  ret = ConvertStringSecurityDescriptorToSecurityDescriptor(
      dacl.str().c_str(),
      SDDL_REVISION_1,
      (PSECURITY_DESCRIPTOR*)&sa.lpSecurityDescriptor,
      NULL
      );
  if (!ret) {
    DWORD last_error = GetLastError();    
    std::cerr << "filed to convert sddl descriptor (" << last_error << ")" 
              << std::endl;
    return 1;
  }

  HANDLE h = CreateFile(
      file_path.c_str(),
      GENERIC_WRITE,
      0, // Plik zablokowany do czasu zamknięcia uchwytu.
      &sa, // Bazowy ACL.
      CREATE_NEW, // Flaga analogiczna do O_CREAT | O_EXCL z GNU/Linux.
      FILE_ATTRIBUTE_NORMAL,
      NULL);

  if (h == INVALID_HANDLE_VALUE) {
    DWORD last_error = GetLastError();
    std::cerr << "error creating file (" << last_error << ")" << std::endl;
    return 1;
  }

  LocalFree(sa.lpSecurityDescriptor);

  // ...

  CloseHandle(h);
  std::cout << "done\n";
  return 0;
}



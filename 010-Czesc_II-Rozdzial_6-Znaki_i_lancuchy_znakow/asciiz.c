#include <stdio.h>
#include <string.h>

int main(void) {
  char my_string[128] = "My name is Alice";
  puts(my_string);

  my_string[11] = '\0';
  puts(my_string);

  // Funkcja strcat jest obecnie uznawana za niebezpieczną, ponieważ nie
  // sprawdza, czy docelowy bufor może pomieścić dodatkowe dane. W tym
  // konkretnym przypadku wiemy oczywiście, że doklejenie trzech znaków nie
  // spowoduje przepełnienia bufora, niemniej jednak w kodzie jakości
  // produkcyjnej powinno się raczej użyć poniższego, bezpiecznego,
  // odpowiednika:
  // strcat_s(my_string, sizeof(my_string), "Eve");
  strcat(my_string, "Eve");
  puts(my_string);

  size_t my_string_len = strlen(my_string);
  size_t my_string_len_manual = 0;
  while (my_string[my_string_len_manual] != 0) {
    my_string_len_manual++;
  }

  printf("Length: %u %u\n", 
      (unsigned int)my_string_len,
      (unsigned int)my_string_len_manual);  

  return 0;
}


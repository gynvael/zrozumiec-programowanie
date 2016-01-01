// Kompilacja i uruchomienie.
// 1. Zainstaluj SDL2 (nagłówki i biblioteki).
// 2. (Windows) Skopiuj SDL2.dll do katalogu wynikowego.
// 3. g++ -Wall -Wextra show_bmp24.cpp -o show_bmp24 -lSDL2
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdint.h>

const int WINDOW_W = 640;
const int WINDOW_H = 480;

// Zamiast tworzyć oddzielne funkcje HelperRead32Bits, HelperRead16Bits itd,
// zdefiniuje jeden szablon, który wygeneruje potrzebne funkcje na żądanie.
// Przy czym należy w tym wypadku uważać, by funkcja na pewno otrzymała
// odpowiedni tym w parametrze (w innym przypadku może odczytać zbyt dużo lub
// zbyt mało danych). ALternatywnie możnaby potworzyć wrappery, które wywołają
// szablon z odpowiednimi parametrami i korzystać tylko z nich.
template<typename T>
bool HelperRead(FILE *f, T *data) {
  // Jeśli jestesmy pewni, że kod działa na platformie little endian, i że 
  // używane przez nas typy używają kompatybilnego kodowania jak docelowe
  // zmienne, możemy się posłużyć poniższą linią (zadziała na x86 z GCC/MSVC):
  //
  //  return fread(data, sizeof(T), 1, f) == 1;
  //
  // W innym wypadku przyjęło się dekodowanie liczby bajt po bajcie i składanie
  // finalnej wartości za pomocą operacji logicznych.
  T temp_value(0);

  // Wczytaj zmienną (little endian).
  for (size_t i = 0; i < sizeof(T); i++) {
    uint8_t single_byte;
    if (fread(&single_byte, 1, 1, f) != 1) {
      return false;
    }

    temp_value |= static_cast<T>(single_byte) << (i * 8);
  }

  *data = temp_value;
  return true;
}

SDL_Surface *MyLoadBMP_RGB24(FILE *f, uint32_t offset,
                             int32_t w, int32_t h, bool bottom_up);

SDL_Surface *MyLoadBMP(const char *filename) {
  SDL_Surface *surface = NULL;

  FILE *f = fopen(filename, "rb");
  if (f == NULL) {
    fprintf(stderr, "Error: could not open file %s.\n", filename);
    return NULL;
  }

  // Wczytaj BITMAPFILEHEADER.
  struct BITMAPFILEHEADER_st {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
  } bfh;

  if (!(HelperRead(f, &bfh.bfType) &&
        HelperRead(f, &bfh.bfSize) &&
        HelperRead(f, &bfh.bfReserved1) &&
        HelperRead(f, &bfh.bfReserved2) &&
        HelperRead(f, &bfh.bfOffBits))) {
    fprintf(stderr, "Error: failed to read BITMAPFILEHEADER.\n");
    fclose(f);
    return NULL;
  }
  // Alternatywnie możnaby nadużyć użyć pragma pack i stworzyć strukturę, która
  // w pamięci będzie miała pola rozmieszczone w identyczny sposób jak nagłówek
  // BITMAPFILEHEADER. Niemniej jednak kod tego typu będzie mniej przenośny (nie
  // każda architektura musi udostępniać wyrównanie do jednego bajtu, które
  // używa się w takich wypadkach).  

  if (bfh.bfType != 0x4d42) {
    fprintf(stderr, "Error: incorrect BMP magic.\n");
    fclose(f);
    return NULL;
  }

  // Wczytaj BITMAPINFOHDEAR.
  struct BITMAPINFOHEADER_st {
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
  } bih;

  if (!(HelperRead(f, &bih.biSize) &&
        HelperRead(f, &bih.biWidth) &&
        HelperRead(f, &bih.biHeight) &&
        HelperRead(f, &bih.biPlanes) &&
        HelperRead(f, &bih.biBitCount) &&
        HelperRead(f, &bih.biCompression) &&
        HelperRead(f, &bih.biSizeImage) &&
        HelperRead(f, &bih.biXPelsPerMeter) &&
        HelperRead(f, &bih.biYPelsPerMeter) &&
        HelperRead(f, &bih.biClrUsed) &&
        HelperRead(f, &bih.biClrImportant))) {
    fprintf(stderr, "Error: failed to read BITMAPINFOHEADER.\n");
    fclose(f);
    return NULL;
  }

  // Odrzuć bitmapy których szerokość ma ekstremalne wielkości.
  // Note: liczba 20000 * 20000 * 4 nadal mieści się w uint32_t (więc nie
  // dojdzie do przepełnienia).
  if (bih.biWidth <= 0 || bih.biWidth > 20000) {
    fprintf(stderr, "Error: sanity check on biWidth failed.\n");
    fclose(f);
    return NULL;
  }

  bool bottom_up = true;

  // Note: Używam 64-bitowej zmiennej typu całkowitego, ponieważ inaczej mogłoby
  // dojść do przepełnienia jeśli height byłoby równe INT_MIN. Alternatywnie
  // mógłbym sprawdzić czy bih.biWidth jest równe INT_MIN i wyjść z błędem w
  // takim wypadku.
  int64_t height = bih.biHeight;
  if (height < 0) {
    bottom_up = false;
    height = -height;
  }

  if (height == 0 || height > 20000) {
    fprintf(stderr, "Error: sanity check on biHeight failed.\n");
    fclose(f);
    return NULL;
  }
  bih.biHeight = static_cast<uint32_t>(height);

  // Wywołaj odpowiednią funkcję obsługującą dane kodowanie.
  if (bih.biCompression == 0 /* BI_RGB */ && bih.biBitCount == 24) {
    surface = MyLoadBMP_RGB24(
        f, bfh.bfOffBits, bih.biWidth, bih.biHeight, bottom_up);
  } else {
    fprintf(stderr, "Error: BMP type not supported.\n");
  }

  // Done.
  fclose(f);
  return surface;
}

SDL_Surface *MyLoadBMP_RGB24(FILE *f, uint32_t offset,
                             int32_t w, int32_t h, bool bottom_up) {
  // Stwórz surface SDL.
  SDL_Surface *surface = SDL_CreateRGBSurface(
      0, w, h, 24, 
      0xff0000, 0x00ff00, 0x0000ff, 0); // RGB0

  if (surface == NULL) {
    fprintf(stderr, "Error: failed to create an RGB surface.\n");
    return NULL;
  }

  // Oblicz pitch.
  uint32_t pitch = (w * 3 + 3) & ~3U;
  
  // Wczytaj wiersze.
  int32_t y = bottom_up ? h - 1 : 0;
  int32_t dy = bottom_up ? -1 : 1;  // Kolejność przetwarzania wierszy.
  int32_t input_y = 0;
  uint8_t *pixels = static_cast<uint8_t *>(surface->pixels);

  while (y >= 0 && y < h) {
    // Przesuń kursor odczytu na odpowiedni wiersz z danymi (i przy okazji
    // pomiń padding jeśli takowy jest).
    uint32_t data_offset = offset + pitch * input_y;
    long current_offset = ftell(f);
    if (current_offset == -1) {
      fprintf(stderr, "Error: ftell failed.\n");
      SDL_FreeSurface(surface);
      return NULL;
    }

    if (static_cast<uint32_t>(current_offset) != data_offset) {
      // Wywołaj seek tylko jeśli jest to wymagane.
      if (fseek(f, data_offset, SEEK_SET) != 0) {
        fprintf(stderr, "Error: seek failed.\n");
        SDL_FreeSurface(surface);
        return NULL;
      }
    }

    // Wczytaj wiersz od razu do docelowego surface.
    if (fread(&pixels[y * surface->pitch], w * 3, 1, f) != 1) {
      fprintf(stderr, "Error: read of pixel data failed.\n");
      SDL_FreeSurface(surface);
      return NULL;
    }
    
    // Przejdz do kolejnego wiersza.
    y += dy;
    input_y++;
  }

  // Done.
  return surface;
}

// SDL main ma zwyczaj podmieniać stdin/stdout, czego osobiście nie lubie.
// Poniższe usunięcie makra main rozwiązuje ten problem (niestety może to
// również mieć inne efekty uboczne w niektórych przypadkach).
#undef main
int main() {
  // Stwórz okno o rozmiarze 640x480.
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window = SDL_CreateWindow("BMP loader",
      SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      WINDOW_W, WINDOW_H,
      0);

  SDL_Surface *surface = SDL_GetWindowSurface(window);
  SDL_Surface *image = MyLoadBMP("test.bmp");
  if (image == NULL) {
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  // Skopiuj (blit) surface z bitmapą na środek bufora klatki (a raczej bufora
  // okna).
  SDL_Rect pos = {
    (WINDOW_W - image->w) / 2, (WINDOW_H - image->h) / 2, image->w, image->h
  };
  SDL_BlitSurface(image, NULL, surface, &pos);

  // Przerysuj okno (tj. wyświetl bufor klatki).
  SDL_UpdateWindowSurface(window);

  // Poczekaj aż okno zostanie zamknięte lub zostanie naciśnięty przycisk ESC.
  bool shutdown = false;
  while (!shutdown) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if ((event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) ||
          event.type == SDL_QUIT) {
        shutdown = true;
        break;
      }
    }
  }

  // Koniec.
  SDL_FreeSurface(image);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}


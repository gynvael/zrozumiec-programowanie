#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdint.h>

const int WINDOW_W = 640;
const int WINDOW_H = 480;

// SDL main ma zwyczaj podmieniać stdin/stdout, czego osobiście nie lubie.
// Poniższe usunięcie makra main rozwiązuje ten problem (niestety może to
// również mieć inne efekty uboczne w niektórych przypadkach).
#undef main
int main() {
  // Stwórz okno o rozmiarze 640x480.
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window = SDL_CreateWindow("gradient",
      SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      WINDOW_W, WINDOW_H,
      0);

  SDL_Surface *surface = SDL_GetWindowSurface(window);

  // Użyjemy 24-bitowego surface z SDL zamiast tablicy (jak w przypadku
  // poprzedniego przykładu w języku Python).
  const int W = 256;
  const int H = 256;
  SDL_Surface *gradient = SDL_CreateRGBSurface(
      0, W, H, 24, 
      // Linia poniżej zawiera następującą maskę kolorów:
      // R zajmuje bajt 0 (bit 0).
      // G zajmuje bajt 1 (bit 8).
      // B zajmuje bajt 2 (bit 16).
      // Nie ma kanału ALPHA.
      // (Należy pamiętać, że maska zależy również od endian na danej
      // architekturze).
      0xff << 0, 0xff << 8, 0xff << 16, 0);

  // Narysuj czerwono-czarny gradient na „surowej” bitmapie (o której w tym
  // wypadku można myśleć jak o tablicy bajtów).
  uint8_t *pixels = static_cast<uint8_t*>(gradient->pixels);
  for (int y = 0; y < H; y++) {
    for (int x = 0; x < W; x++) {
      pixels[x * 3 + y * gradient->pitch + 0] = x;  // R
      pixels[x * 3 + y * gradient->pitch + 1] = 0;  // G
      pixels[x * 3 + y * gradient->pitch + 2] = 0;  // B

      // Powyższe można również zapisać w następujący sposób:
      // #pragma pack(push, 1)
      // struct pixel_st {
      //   uint8_t r, g, b;
      // } *pixel = reinterpret_cast<pixel_st*>(
      //     &pixels[x * 3 + y * gradient->pitch]);
      // #pragma pack(pop)
      // pixel->r = x;
      // pixel->g = 0;
      // pixel->b = 0;
    }
  }

  // Skopiuj (blit) powyższy surface na środek bufora klatki (a raczej bufora
  // okna).
  SDL_Rect pos = {
    (WINDOW_W - W) / 2, (WINDOW_H - H) / 2, W, H
  };
  SDL_BlitSurface(gradient, NULL, surface, &pos);

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
  SDL_FreeSurface(gradient);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}


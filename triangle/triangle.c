#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <SDL2/SDL.h>

int main(int argc, const char *argv[]) {
  SDL_Init(SDL_INIT_EVERYTHING);

  SDL_Window *window = SDL_CreateWindow(
      "Minimal",
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      640,
      480,
      SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI
  );
  SDL_GLContext context = SDL_GL_CreateContext(window);

  // GL stuff
  glClearColor(
      0x49 / 255.0,
      0x36 / 255.0,
      0x57 / 255.0,
      1
  ); // Set the clear color to #493657

  // Main loop
  SDL_bool running = SDL_TRUE;
  SDL_Event event;
  while (running) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT:
          running = SDL_FALSE;
          break;
        default:
          continue;
      }
    }

    glClear(GL_COLOR_BUFFER_BIT); // Clear the background with color
    SDL_GL_SwapWindow(window);
    // Delay so that there's at least some time between frames
    SDL_Delay(1);
  }

  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}

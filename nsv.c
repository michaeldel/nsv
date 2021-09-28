#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>

#define INITIAL_WIDTH 640
#define INITIAL_HEIGHT 320

int main(void) {
    int err = SDL_Init(SDL_INIT_VIDEO);
    if (err) {
        fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
        return EXIT_FAILURE; /* TODO: UNIX exit codes */
    }

    SDL_Window * window = SDL_CreateWindow(
        "nsv",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        INITIAL_WIDTH, INITIAL_HEIGHT,
        SDL_WINDOW_OPENGL
    );

    if (window == NULL) {
        fprintf(stderr, "Unable to create window: %s\n", SDL_GetError());
        return EXIT_FAILURE; /* TODO: UNIX exit codes */
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}

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

    SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (renderer == NULL) {
        fprintf(stderr, "Unable to create renderer: %s\n", SDL_GetError());
        return EXIT_FAILURE; /* TODO: UNIX exit codes */
    }

    const int width = INITIAL_WIDTH;
    const int height = INITIAL_HEIGHT;
    const Uint32 pixelformat = SDL_PIXELFORMAT_RGBA8888;

    SDL_Texture * texture = SDL_CreateTexture(
        renderer,
        pixelformat,
        SDL_TEXTUREACCESS_STREAMING,
        width,
        height
    );

    if (texture == NULL) {
        fprintf(stderr, "Unable to create texture: %s\n", SDL_GetError());
        return EXIT_FAILURE; /* TODO: UNIX exit codes */
    }

    SDL_PixelFormat * format = SDL_AllocFormat(pixelformat);

    if (format == NULL) {
        fprintf(stderr, "Unable to allocate format: %s\n", SDL_GetError());
        return EXIT_FAILURE; /* TODO: UNIX exit codes */
    }

    for (;;) {
        SDL_Event event;
        SDL_PollEvent(&event);

        if (event.type == SDL_QUIT) break;

        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
    }

    SDL_FreeFormat(format);

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return EXIT_SUCCESS;
}

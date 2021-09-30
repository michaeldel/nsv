#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>

#define INITIAL_WIDTH 1280
#define INITIAL_HEIGHT 320

#define MAX_SEQ_LEN 10000

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

size_t readsequence(int * sequence) {
    size_t len = 0;
    int input;

    while (scanf("%d", &input) > 0)
        sequence[len++] = input;

    assert(len <= MAX_SEQ_LEN);

    return len;
}

int main(void) {
    int sequence[MAX_SEQ_LEN]; /* TODO: longer sequences */
    const size_t sequencelen = readsequence(sequence);

    for (size_t i = 0; i < sequencelen; i++)
        printf("%d ", sequence[i]);
    putchar('\n');

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

    const unsigned int width = 1024;
    const unsigned int height = 64;
    const Uint32 pixelformat = SDL_PIXELFORMAT_RGB888;

    assert(width <= INT_MAX);
    assert(height <= INT_MAX);

    SDL_Texture * texture = SDL_CreateTexture(
        renderer,
        pixelformat,
        SDL_TEXTUREACCESS_STREAMING,
        (int) width,
        (int) height
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

    int pitch = width * 3;
    void * tmp;

    SDL_LockTexture(texture, NULL, &tmp, &pitch);
    Uint32 * pixels = tmp;

    for (size_t y = 0; y < height; y++) 
        for (size_t x = 0; x < width; x++) {
            const unsigned long long num = (unsigned long long) sequence[x];
            const unsigned int ison = (num >> (height - y - 1)) & 0x1 ? 1 : 0;

            const Uint8 r = ison * 255;
            const Uint8 g = ison * 255;
            const Uint8 b = ison * 255;

            pixels[y * width + x] = SDL_MapRGB(format, r, g, b);
        }

    SDL_UnlockTexture(texture);

    const unsigned int winwidth = INITIAL_WIDTH;
    const unsigned int winheight = INITIAL_HEIGHT;
    
    int offset = 0;

    for (;;) {
        SDL_Event event;
        SDL_WaitEvent(&event);

        if (event.type == SDL_QUIT) break;
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_q) break;

        /* TODO: check limits correctly */
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_h)
            offset = MAX(0, offset - 1);

        /* TODO: check limits correctly */
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_l)
            offset = MIN((int) (winwidth - width), offset + 1);

        SDL_RenderClear(renderer);
        SDL_Rect src = { offset, 0, winwidth / 4, height };
        SDL_Rect dest = { 0, 0, winwidth, winheight };
        SDL_RenderCopy(renderer, texture, &src, &dest);
        SDL_RenderPresent(renderer);
    }

    SDL_FreeFormat(format);

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return EXIT_SUCCESS;
}

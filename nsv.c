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

int seqmax(const int * sequence, size_t len) {
    assert(len > 0);

    int result = sequence[0];

    for (size_t i = 1; i < len; i++)
        result = MAX(result, sequence[i]);

    return result;
}

struct viewports {
    SDL_Rect main;
    SDL_Rect minimap;
};

int main(void) {
    int sequence[MAX_SEQ_LEN]; /* TODO: longer sequences */
    const size_t sequencelen = readsequence(sequence);

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
    
    const unsigned int minimapwidth = sequencelen;
    const unsigned int minimapheight = 160;

    SDL_Texture * minimap = SDL_CreateTexture(
        renderer,
        pixelformat,
        SDL_TEXTUREACCESS_TARGET,
        (int) minimapwidth,
        (int) minimapheight
    );

    SDL_SetRenderTarget(renderer, minimap);
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

    const int ceil = seqmax(sequence, sequencelen);

    for (size_t x = 0; x < minimapwidth; x++) {
        const int lineheight = sequence[x] * minimapheight / ceil;
        if (lineheight == 0) continue;

        const int y1 = minimapheight - 1;
        const int y2 = minimapheight - 1 - lineheight;

        SDL_RenderDrawLine(renderer, x, y1, x, y2);
    }

    SDL_SetRenderTarget(renderer, NULL);
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);

    const struct viewports viewports = {
        .main =    { 0, 0, winwidth, winheight - 64 },
        .minimap = { 0, winheight - 64 + 1, winwidth, 64 },
    };

    unsigned int zoom = 4;

    int xoffset = 0;
    int yoffset = 0;

    for (;;) {
        SDL_Event event;
        SDL_WaitEvent(&event);

        if (event.type == SDL_QUIT) break;
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_q) break;

        /* TODO: check limits correctly */
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_h)
            xoffset = MAX(0, xoffset - 1);

        /* TODO: check limits correctly */
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_l)
            xoffset = MIN((int) (winwidth - width), xoffset + 1);

        /* TODO: check limits correctly */
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_j)
            yoffset = MAX(0, yoffset - 1);

        /* TODO: check limits correctly */
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_k)
            yoffset = MIN(64, yoffset + 1); /* TODO: bignum, other bases, etc */

        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_MINUS)
            zoom = MAX(1, zoom - 1);
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_EQUALS)
            zoom = MIN(viewports.main.h, (int) (zoom + 1));

        /* TODO: can be optimized by redrawing only on relevant event */
        SDL_RenderClear(renderer);
        SDL_Rect src = {
            xoffset,
            64 - yoffset - viewports.main.h / zoom,/* TODO: bignum, other bases, etc */
            viewports.main.w / zoom, viewports.main.h / zoom
        };
        SDL_RenderCopy(renderer, texture, &src, &viewports.main);
        SDL_RenderCopy(renderer, minimap, NULL, &viewports.minimap);
        SDL_RenderPresent(renderer);
    }

    SDL_FreeFormat(format);

    SDL_DestroyTexture(texture);
    SDL_DestroyTexture(minimap);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return EXIT_SUCCESS;
}

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <gmp.h>
#include <SDL2/SDL.h>

#define INITIAL_WIDTH 1280
#define INITIAL_HEIGHT 320

#define MAX_SEQ_LEN 10000

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

size_t readsequence(mpz_t * sequence) {
    size_t len = 0;

    mpz_t num;
    mpz_init(num);

    char buffer[BUFSIZ];
    char format[256];
    sprintf(format, "%%%ds", BUFSIZ - 1);

    while (scanf(format, buffer) > 0) {

        mpz_set_str(num, buffer, 10);
        mpz_init_set(sequence[len], num);
        len++;
    }

    assert(len <= MAX_SEQ_LEN);

    mpz_clear(num);

    return len;
}

size_t seqmaxbitsize(mpz_t * sequence, size_t len) {
    assert(len > 0);

    size_t result = 0;
    const int base = 2;

    for (size_t i = 0; i < len; i++)
        result = MAX(result, mpz_sizeinbase(sequence[i], base));

    return result;
}

struct viewports {
    SDL_Rect main;
    SDL_Rect minimap;
};

void updateviewports(struct viewports * viewports, int winwidth, int winheight) {
    viewports->main =    (SDL_Rect) { 0, 0, winwidth, winheight - 64 };
    viewports->minimap = (SDL_Rect) { 0, winheight - 64 + 1, winwidth, 64 };
}

int main(void) {
    mpz_t sequence[MAX_SEQ_LEN]; /* TODO: longer sequences */
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
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL
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

    const unsigned int width = MAX(1024, sequencelen);
    const unsigned int height = MAX(64, seqmaxbitsize(sequence, sequencelen));
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
            mpz_t num;
            mpz_init_set(num, sequence[x]);
            const mp_bitcnt_t bitindex = height - y - 1;
            const unsigned int ison = mpz_tstbit(num, bitindex);

            const Uint8 r = ison * 255;
            const Uint8 g = ison * 255;
            const Uint8 b = ison * 255;

            pixels[y * width + x] = SDL_MapRGB(format, r, g, b);
        }

    SDL_UnlockTexture(texture);

    int winwidth;
    int winheight;

    SDL_GetWindowSize(window, &winwidth, &winheight);
    
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

    const unsigned int ceilbits = seqmaxbitsize(sequence, sequencelen);

    for (size_t x = 0; x < minimapwidth; x++) {
        mpz_t tmp;
        mpz_init(tmp);
        mpz_mul_ui(tmp, sequence[x], minimapheight);
        mpz_fdiv_q_2exp(tmp, tmp, ceilbits);

        assert(mpz_fits_sint_p(tmp));

        const int lineheight = mpz_get_si(tmp);
        if (lineheight == 0) continue;

        const int y1 = minimapheight - 1;
        const int y2 = minimapheight - 1 - lineheight;

        SDL_RenderDrawLine(renderer, x, y1, x, y2);

        mpz_clear(tmp);
    }

    SDL_SetRenderTarget(renderer, NULL);
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);

    struct viewports viewports;
    updateviewports(&viewports, winwidth, winheight);

    unsigned int zoom = 4;

    int xoffset = 0;
    int yoffset = 0;

    for (;;) {
        SDL_Event event;
        SDL_WaitEvent(&event);

        if (event.type == SDL_QUIT) break;
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_q) break;

        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_h)
            xoffset = MAX(0, xoffset - 1);
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_l)
            xoffset = MIN((int) width - 1, xoffset + 1); /* TODO: account for zoom */

        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_j)
            yoffset = MAX(0, yoffset - 1);
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_k)
            yoffset = MIN((int) height - 1, yoffset + 1); /* TODO: account for zoom */

        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_MINUS)
            zoom = MAX(1, zoom - 1);
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_EQUALS)
            zoom = MIN(viewports.main.h, (int) (zoom + 1));

        /* TODO: can be optimized by redrawing only on relevant event */
        SDL_GetWindowSize(window, &winwidth, &winheight);
        struct viewports viewports;
        updateviewports(&viewports, winwidth, winheight);

        SDL_RenderClear(renderer);
        const SDL_Rect src = {
            MAX(0, xoffset), /* TODO: account for zoom */
            MAX(0, (int) (height - viewports.main.h / zoom - yoffset)),
            MIN(width, viewports.main.w / zoom),
            MIN(height, viewports.main.h / zoom)
        };
        const SDL_Rect dest = {
            viewports.main.x,
            viewports.main.y + MAX(0, (int) (viewports.main.h - src.h * zoom)),
            src.w * zoom,
            src.h * zoom,
        };
        SDL_RenderCopy(renderer, texture, &src, &dest);
        SDL_RenderCopy(renderer, minimap, NULL, &viewports.minimap);

        const SDL_Rect minimaplocation = {
            viewports.minimap.x + xoffset * viewports.minimap.w / sequencelen,
            viewports.minimap.y,
            src.w,
            viewports.minimap.h,
        };
        SDL_SetRenderDrawColor(
            renderer, 0xFF, 0xFF, 0xFF, (SDL_ALPHA_OPAQUE + SDL_ALPHA_TRANSPARENT) / 2
        );
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderFillRect(renderer, &minimaplocation);

        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, SDL_ALPHA_OPAQUE);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        SDL_RenderPresent(renderer);
    }

    SDL_FreeFormat(format);

    SDL_DestroyTexture(texture);
    SDL_DestroyTexture(minimap);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    for (size_t i = 0; i < sequencelen; i++)
        mpz_clear(sequence[i]);

    return EXIT_SUCCESS;
}

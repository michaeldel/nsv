#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <gmp.h>
#include <SDL2/SDL.h>

#include "sequence.h"
#include "textures.h"

#define INITIAL_WIDTH 1280
#define INITIAL_HEIGHT 720

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

size_t maxbitsize(mpz_t * numbers, size_t len) {
    assert(len > 0);

    size_t result = 0;
    const int base = 2;

    for (size_t i = 0; i < len; i++)
        result = MAX(result, mpz_sizeinbase(numbers[i], base));

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
    struct sequence * sequence = readsequencefromstdin();

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

    const unsigned int width = MAX(1024, sequence->len);
    const unsigned int height = MAX(64, maxbitsize(sequence->numbers, sequence->len));
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

    filltexturewithsequence(texture, format, sequence, 2);

    int winwidth;
    int winheight;

    SDL_GetWindowSize(window, &winwidth, &winheight);
    
    const unsigned int minimapwidth = sequence->len;
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

    const unsigned int ceilbits = maxbitsize(sequence->numbers, sequence->len);

    for (size_t x = 0; x < minimapwidth; x++) {
        mpz_t tmp;
        mpz_init(tmp);
        mpz_mul_ui(tmp, sequence->numbers[x], minimapheight);
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

    unsigned int zoom = 1;

    int xoffset = 0;
    int yoffset = 0;

    char textinput[BUFSIZ] = { 0 };

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

        if (event.type == SDL_TEXTINPUT)
            strcat(textinput, event.text.text); /* TODO: bounds check */

        const size_t textinputlen = strlen(textinput);

        if (textinput[textinputlen - 1] == 'G') {
            textinput[0] = '\0';
            xoffset = width - 1;
        }
        if (textinput[textinputlen - 1] == 'g' && textinput[textinputlen - 2] == 'g') {
            textinput[0] = '\0';
            xoffset = 0;
        }
        /* TODO: better size handling than this nasty hack */
        textinput[0] = textinput[textinputlen - 1];
        textinput[1] = '\0';

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
            viewports.minimap.x + xoffset * viewports.minimap.w / sequence->len,
            viewports.minimap.y,
            viewports.minimap.w * src.w / sequence->len,
            viewports.minimap.h,
        };
        SDL_SetRenderDrawColor(
            renderer, 0xFF, 0xFF, 0xFF, (SDL_ALPHA_OPAQUE + SDL_ALPHA_TRANSPARENT) / 4
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

    delsequence(sequence);

    return EXIT_SUCCESS;
}

#include <assert.h>
#include <stdlib.h>

#include <gmp.h>
#include <SDL2/SDL.h>

#include "sequence.h"

void filltexturewithsequence(
    SDL_Texture * texture,
    SDL_PixelFormat * format,
    const struct sequence * sequence,
    uint8_t base
) {
    assert(base == 2); /* TODO: more bases */

    int w, h, access;
    SDL_QueryTexture(texture, NULL, &access, &w, &h);

    assert(access == SDL_TEXTUREACCESS_STREAMING);
    const unsigned int width = (unsigned int) w;
    const unsigned int height = (unsigned int) h;

    int pitch = width * format->BytesPerPixel;
    void * tmp;

    SDL_LockTexture(texture, NULL, &tmp, &pitch);
    Uint32 * pixels = tmp; 

    for (size_t y = 0; y < height; y++) 
        for (size_t x = 0; x < width; x++) {
            mpz_t num;
            mpz_init_set(num, sequence->numbers[x]);
            const mp_bitcnt_t bitindex = height - y - 1;
            const unsigned int ison = mpz_tstbit(num, bitindex);

            const Uint8 r = ison * 255;
            const Uint8 g = ison * 255;
            const Uint8 b = ison * 255;

            pixels[y * width + x] = SDL_MapRGB(format, r, g, b);
        }

    SDL_UnlockTexture(texture);
}

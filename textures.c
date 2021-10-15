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
    assert(base >= 1);

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
            unsigned int digit;

            if (base != 2) {
                const unsigned int digitindex = height - y - 1;

                mpz_t powofbase, quotient;
                mpz_inits(powofbase, quotient, NULL);

                mpz_ui_pow_ui(powofbase, base, digitindex);
                mpz_fdiv_q(quotient, sequence->numbers[x], powofbase);

                digit = mpz_mod_ui(quotient, quotient, base);
                mpz_clears(powofbase, quotient);
            } else {
                const mp_bitcnt_t bitindex = height - y - 1;
                digit = mpz_tstbit(sequence->numbers[x], bitindex);
            }

            /* TODO: color map */
            const Uint8 r = digit * 255 / (base - 1);
            const Uint8 g = digit * 255 / (base - 1);
            const Uint8 b = digit * 255 / (base - 1);

            pixels[y * width + x] = SDL_MapRGB(format, r, g, b);
        }

    SDL_UnlockTexture(texture);
}

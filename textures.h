#ifndef TEXTURES_H
#define TEXTURES_H

#include <stdint.h>

#include <SDL2/SDL.h>

#include "sequence.h"

void filltexturewithsequence(
    SDL_Texture * texture,
    SDL_PixelFormat * format,
    const struct sequence * sequence,
    uint8_t base
);

#endif

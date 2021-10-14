#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "gmp.h"

struct sequence {
    mpz_t * numbers;
    size_t len;
    size_t capacity;
};

struct sequence * mksequence(void);
void delsequence(struct sequence * sequence);

struct sequence * readsequencefromstdin(void);

#endif

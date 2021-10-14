#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <gmp.h>

#include "sequence.h"

#define INITIAL_CAPACITY 1024

struct sequence * mksequence(void) {
    struct sequence * sequence = malloc(sizeof(struct sequence));
    if (!sequence) {
        perror("Failed to allocate sequence\n");
        abort();
    }

    sequence->len = 0;
    sequence->capacity = INITIAL_CAPACITY;
    sequence->numbers = calloc(sequence->capacity, sizeof(mpz_t));
    if (!sequence->numbers) {
        perror("Failed to allocate sequence numbers\n");
        abort();
    }

    return sequence;
}

void delsequence(struct sequence * sequence) {
    for (size_t i = 0; i < sequence->len; i++)
        mpz_clear(sequence->numbers[i]);
    free(sequence->numbers);
    free(sequence);
}

static void pushnumber(struct sequence * sequence, mpz_t number) {
    if (sequence->len == sequence->capacity) {
        sequence->capacity *= 2;
        sequence->numbers = reallocarray(
            sequence->numbers, sequence->capacity, sizeof(mpz_t)
        );

        if (!sequence->numbers) {
            perror("Failed to allocate sequence numbers\n");
            abort();
        }
    }

    mpz_init_set(sequence->numbers[sequence->len], number);
    sequence->len++;
}

struct sequence * readsequencefromstdin(void) {
    struct sequence * sequence = mksequence();
    
    char buffer[BUFSIZ];
    char format[256];
    sprintf(format, "%%%ds", BUFSIZ - 1);

    mpz_t num;
    mpz_init(num);

    while (scanf(format, buffer) > 0) {
        mpz_set_str(num, buffer, 10);
        pushnumber(sequence, num);
    }

    mpz_clear(num);

    return sequence;
}

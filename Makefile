CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic
LDFLAGS = -lgmp -lSDL2

nsv: nsv.c sequence.c sequence.h
	$(CC) $(CFLAGS) -o $@ $(LDFLAGS) $^

clean:
	rm nsv

.PHONY: clean


CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic
LDFLAGS = -lSDL2

nsv: nsv.c
	$(CC) $(CFLAGS) -o $@ $(LDFLAGS) $^

clean:
	rm nsv

.PHONY: clean


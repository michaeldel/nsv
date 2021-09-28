CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic

nsv: nsv.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm nsv

.PHONY: clean


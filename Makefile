.POSIX:
.PHONY: all clean

CC := gcc
CFLAGS := -Wall -Werror -Wextra -Wpedantic -g -O0
SRC := $(wildcard *.c)
OBJ := $(SRC:.c=.o)
HDR := $(wildcard *.h)

all: $(OBJ)

%.o: %.c $(HDR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ)


.POSIX:
.PHONY: all clean run

CC := gcc
CFLAGS := -Wall -Werror -Wextra -Wpedantic -g -O0
SRC := $(wildcard *.c)
OBJ := $(SRC:.c=.o)
HDR := $(wildcard *.h)
EXEC := libconf

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $^ -o $@

%.o: %.c $(HDR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(EXEC) $(OBJ)

run:
	./$(EXEC)

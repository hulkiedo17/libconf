.POSIX:
.PHONY: all clean test

CC := gcc
CFLAGS := -Wall -Werror -Wextra -Wpedantic -g -O0
SRC_DIR := ./src
BIN_DIR := ./build

TEST_DIR := ./test
TEST_SRC := $(wildcard $(TEST_DIR)/*.c)
TEST_EXEC := test

SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(patsubst $(SRC_DIR)/%.c, $(BIN_DIR)/%.c, $(SRC))
OBJ := $(OBJ:.c=.o)
HDR := $(wildcard ./include/*.h)
LIB := $(BIN_DIR)/libconf.a

all: $(LIB)

$(LIB): $(OBJ)
	ar -rcs $@ $^

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c $(HDR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ) $(LIB)

test: $(LIB)
	$(CC) $(CFLAGS) $(TEST_SRC) -o $(TEST_DIR)/$(TEST_EXEC) -L./build -lconf

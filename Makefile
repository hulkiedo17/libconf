.POSIX:

CC := gcc
CFLAGS := -Wall -Werror -Wextra -Wpedantic -g -O0
SRC_DIR := ./src
BIN_DIR := ./build
INC_DIR := ./include

TEST_DIR := ./test
TEST_SRC := $(wildcard $(TEST_DIR)/*.c)
TEST_EXEC := test

SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(patsubst $(SRC_DIR)/%.c, $(BIN_DIR)/%.c, $(SRC))
OBJ := $(OBJ:.c=.o)
HDR := $(wildcard ./include/*.h)
LIB := $(BIN_DIR)/libconf.a

DEST_DIR_LIB := /usr/local/lib
DEST_DIR_INC := /usr/local/include

.PHONY: all
all: $(LIB)

$(LIB): $(OBJ)
	ar -rcs $@ $^

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c $(HDR)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(OBJ) $(LIB)

.PHONY: test
test: $(LIB)
	$(CC) $(CFLAGS) $(TEST_SRC) -o $(TEST_DIR)/$(TEST_EXEC) -L$(BIN_DIR) -lconf -I$(INC_DIR)

.PHONY: install
install:
	cp $(LIB) $(DEST_DIR_LIB)
	cp $(HDR) $(DEST_DIR_INC)
SRC_DIR = src
BUILD_DIR = build

CFLAGS = -Wall -Wextra -Werror -pedantic -O3

SRC_FILES = main

DEPS =

_LIBS = raylib m
LIBS = $(patsubst %,-l%,$(_LIBS))

OBJS = $(patsubst %,$(BUILD_DIR)/%.o,$(SRC_FILES))

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS)
	cc -c $(CFLAGS) $< -o $@

PHONY: all clean run

all: $(OBJS)
	cc $(OBJS) $(CFLAGS) $(LIBS) -o exec

run: all
	./exec

clean:
	rm -rf build/*

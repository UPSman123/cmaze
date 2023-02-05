SRC_DIR = src
BUILD_DIR = build

CFLAGS = -Wall -Wextra -Werror -O3

SRC_FILES = main

DEPS =

_LIBS = raylib m
LIBS = $(patsubst %,-l%,$(_LIBS))

OBJS = $(patsubst %,$(BUILD_DIR)/%.o,$(SRC_FILES))

all: clean $(OBJS)
	cc $(OBJS) $(LIBS) -o exec

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS)
	cc -c $(CFLAGS) $< -o $@

PHONY: all clean run

run: all
	./exec

clean:
	rm -rf build/*

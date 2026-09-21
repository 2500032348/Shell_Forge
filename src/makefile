CC = gcc

CFLAGS = -Wall -Wextra -Iinclude

TARGET = shellforge

SRC = src/main.c \
      src/token.c \
      src/lexer.c \
      src/parser.c \
      src/expand.c \
      src/history.c \
      src/builtin.c \
      src/executor.c

OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
        $(CC) $(OBJ) -o $(TARGET) -lreadline

src/%.o: src/%.c
        $(CC) $(CFLAGS) -c $< -o $@

clean:
        rm -f $(OBJ) $(TARGET)

.PHONY: all clean

CC = gcc

CFLAGS = -Wall -Wextra -Iinclude -g

TARGET = shellforge

SRC = src/main.c \
      src/parser.c \
      src/expand.c

OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean

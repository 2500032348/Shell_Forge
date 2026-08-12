CC = gcc

CFLAGS = -Wall -Wextra -Iinclude

TARGET = shellforge

SRC = src/main.c src/lexer.c src/token.c

OBJ = src/main.o src/lexer.o src/token.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

src/main.o: src/main.c include/lexer.h include/token.h
	$(CC) $(CFLAGS) -c src/main.c -o src/main.o

src/lexer.o: src/lexer.c include/lexer.h include/token.h
	$(CC) $(CFLAGS) -c src/lexer.c -o src/lexer.o

src/token.o: src/token.c include/token.h
	$(CC) $(CFLAGS) -c src/token.c -o src/token.o

clean:
	rm -f $(OBJ) $(TARGET)

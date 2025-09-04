CC = gcc
CFLAGS = -Wall -Wextra -g
SRC = src/main.c src/lexer.c src/interpreter.c src/symbol.c
OBJ = $(SRC:.c=.o)
TARGET = mylang
PROGRAM=programs/program.txt

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET) $(PROGRAM)

clean:
	rm -f $(OBJ) $(TARGET)

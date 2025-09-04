CC = gcc
CFLAGS = -Wall -Wextra -g
SRC = src/main.c src/lexer.c src/parser.c src/ast.c src/interpreter.c src/symbol.c
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
	del /Q src\*.o mylang.exe 2>nul || true

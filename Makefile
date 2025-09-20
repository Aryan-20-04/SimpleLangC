CC = gcc
CFLAGS = -Wall -Wextra -g
SRC = src/main.c src/lexer.c src/parser.c src/ast.c src/interpreter.c src/symbol.c
OBJ = $(SRC:.c=.o)
TARGET = slangc

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET)

run: $(TARGET)
	./$(TARGET) programs/program.slc

install: $(TARGET)
	@echo "Installing $(TARGET) to /usr/local/bin..."
	sudo cp $(TARGET) /usr/local/bin/$(TARGET)
	sudo chmod +x /usr/local/bin/$(TARGET)
	@echo "Installed! You can now run '$(TARGET) filename.slc' from anywhere."

uninstall:
	@echo "Removing $(TARGET) from /usr/local/bin..."
	sudo rm -f /usr/local/bin/$(TARGET)
	@echo "Uninstalled!"

.PHONY: all clean run install uninstall

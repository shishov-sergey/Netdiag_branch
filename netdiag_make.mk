CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = netdiag
SOURCES = netdiag.c
OBJECTS = $(SOURCES:.c=.o)

.PHONY: all clean install

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
	rm -rf netdiag_collect_*

install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/

test: $(TARGET)
	@echo "Running tests..."
	@./$(TARGET) show interfaces
	@echo ""
	@./$(TARGET) show routes
	@echo ""
	@./$(TARGET) show vlans

help:
	@echo "Available targets:"
	@echo "  all     - Build the program (default)"
	@echo "  clean   - Remove build files"
	@echo "  install - Install to /usr/local/bin"
	@echo "  test    - Run basic tests"
	@echo "  help    - Show this help"
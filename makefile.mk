CC = gcc
CFLAGS = -Wall -Wextra -std=gnu99 -D_GNU_SOURCE
SRCDIR = src
OBJDIR = obj
BINDIR = .
TARGET = netdiag

SOURCES = $(SRCDIR)/main.c $(SRCDIR)/interfaces.c $(SRCDIR)/routes.c \
          $(SRCDIR)/vlans.c $(SRCDIR)/check.c $(SRCDIR)/collect.c
OBJECTS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOURCES))

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) $(TARGET)

.PHONY: all clean
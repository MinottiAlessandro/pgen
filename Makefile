# Makefile for C project

# Compiler: use CC environment variable, fallback to gcc
CC ?= gcc

# Compiler flags
CFLAGS = -pthread -Wall -Wextra -O3 -march=native -pthread -flto -ffast-math -funroll-loops

# Target executable name
TARGET = pgen

# Source file
SRC = pgen.c

# Installation directory
PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin

# Default target
.PHONY: all
all: build

# Build step
.PHONY: build
build: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Install step
.PHONY: install
install: $(TARGET)
	install -d $(BINDIR)
	install -m 755 $(TARGET) $(BINDIR)/$(TARGET)

# Clean step
.PHONY: clean
clean:
	rm -f $(TARGET)

# Uninstall step
.PHONY: uninstall
uninstall:
	rm -f $(BINDIR)/$(TARGET)

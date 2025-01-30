# IFJ24
# @brief Makefile for IFJ24 project

# Operating system detection
ifeq ($(OS),Windows_NT)
    DETECTED_OS := Windows
else
    DETECTED_OS := $(shell uname -s)
endif

# Compiler and flags
CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -pedantic -g

# Executable names
ifeq ($(DETECTED_OS),Windows)
    TARGET = ifj24.exe
else
    TARGET = ifj24
endif

# Source files
SOURCES = main.c scanner.c token.c error_codes.c dstring.c file.c \
          parser.c pars_expr.c prec_stack.c stack.c symtable.c generator.c

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Phony targets
.PHONY: all clean memcheck

# Default target (build program)
all: $(TARGET)

# Rule for building the target program
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

# Rule for compiling source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Cleanup
clean:
ifeq ($(DETECTED_OS),Windows)
	if exist *.o del *.o
	if exist $(TARGET) del $(TARGET)
else
	rm -f *.o $(TARGET)
endif

# Target to check for memory leaks using Valgrind
memcheck: $(TARGET)
ifeq ($(DETECTED_OS),Windows)
	@echo "Valgrind is not supported on Windows"
else
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET)
endif

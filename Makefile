# Compiler
CC = clang

# Flags for the compiler
CFLAGS = -O3 -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL -lm -lpthread

# Source files
SRCS = main.c $(wildcard src/*.c)

# Library
LIBS = lib/libraylib.a

# Output executable
OUT = main

# Rules
all: $(OUT)

$(OUT): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) $(LIBS) -o $(OUT)

clean:
	rm -f $(OUT)

run:
	./$(OUT)

.PHONY: all clean


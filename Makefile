CC = clang
CFLAGS = -std=c11 -O3 -g -Wall -Wextra -Wpedantic -Wstrict-aliasing
CFLAGS += -Wno-pointer-arith -Wno-newline-eof -Wno-unused-parameter -Wno-gnu-statement-expression
CFLAGS += -Wno-gnu-compound-literal-initializer -Wno-gnu-zero-variadic-macro-arguments
CFLAGS += -DGL_SILENCE_DEPRECATION

GLFW_PATH = /opt/homebrew/Cellar/glfw/3.4
CFLAGS += -I$(GLFW_PATH)/include 
LDFLAGS = -L$(GLFW_PATH)/lib -lglfw -framework OpenGL

SRC = src/main.c
OBJ = $(SRC:src/%.c=bin/%.o)	
EXEC = bin/main.exe

.PHONY: all clean

main.exe: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) $(LDFLAGS)

bin/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(EXEC) $(OBJ)
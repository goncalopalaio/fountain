# Flags
CFLAGS = -pedantic -O2  -Wno-deprecated
LIBS :=
CC= g++


ifeq ($(OS),Windows_NT)
BIN := $(BIN).exe
	LIBS := -lglfw -lopengl32 -lm -lGLU32 -lGLEW32
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		LIBS := -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo -lm -lGLEW `pkg-config --cflags --libs glfw3` `pkg-config --cflags --libs GLEW`
	else
		LIBS := -lglfw -lGL -lm -lGLU -lGLEW
	endif
endif

all: generate main

generate: clean
ifeq ($(OS),Windows_NT)
	@mkdir bin 2> nul || exit 0
else
	@mkdir -p bin	
endif

clean:
	@rm -rf bin

main: generate
	$(CC) $(CFLAGS) -o main main.cpp $(LIBS)

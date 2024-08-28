# Compiler
CXX = g++

# Compiler Flags
CXXFLAGS = -Wall -std=c++17 `sdl2-config --cflags`

# Linker Flags (for SDL2)
LDFLAGS = `sdl2-config --libs`

# Source files
SRC = main.cpp \
      init.cpp \
      draw.cpp \
	  struct.cpp \

# Object files
OBJ = $(SRC:.cpp=.o)

# Executable name
EXEC = sdl_main

# Default target
all: $(EXEC)

# Linking the executable
$(EXEC): $(OBJ)
	$(CXX) $(OBJ) -o $(EXEC) $(LDFLAGS)

# Compiling each .cpp file into .o files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up object files and executable
clean:
	rm -f $(OBJ) $(EXEC)

# Phony targets
.PHONY: all clean
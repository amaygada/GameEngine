# Compiler
CXX = g++

# Compiler Flags
CXXFLAGS = -Wall -std=c++17 -I./src/include

# Linker Flags (for SDL2)
LDFLAGS = -L./src/lib -lSDL2main -lSDL2

SUBSYSTEM = ./src/subsystem
UTILS = ./src/utils

# Source files
SRC = main.cpp \
	$(SUBSYSTEM)/rendering/render.cpp \
	$(SUBSYSTEM)/input_handling/input.cpp \
	$(SUBSYSTEM)/physics/physics.cpp \
	$(SUBSYSTEM)/animation/animation.cpp \
	$(SUBSYSTEM)/collision/collision.cpp \
	$(UTILS)/app.cpp \
	$(UTILS)/entity.cpp \
	$(UTILS)/timer.cpp \

# Object files
OBJ = $(SRC:.cpp=.o)

# Executable name
EXEC = main

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
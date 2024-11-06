# Compiler
CXX = g++

# Compiler Flags
CXXFLAGS = -Wall -std=c++17 -I./src/include $(shell sdl2-config --cflags)

# Linker Flags (for SDL2)
LDFLAGS = $(shell sdl2-config --libs) -lSDL2main -lSDL2 -lzmq -lpthread

SUBSYSTEM = ./src/subsystem
UTILS = ./src/utils

# Source files
SRC = main.cpp \
	$(SUBSYSTEM)/rendering/render.cpp \
	$(SUBSYSTEM)/input_handling/input.cpp \
	$(SUBSYSTEM)/physics/physics.cpp \
	$(SUBSYSTEM)/animation/animation.cpp \
	$(SUBSYSTEM)/collision/collision.cpp \
	$(SUBSYSTEM)/connection/client.cpp \
	$(SUBSYSTEM)/connection/server.cpp \
	$(SUBSYSTEM)/event_manager/event_manager.cpp \
	$(SUBSYSTEM)/connection/p2pclient.cpp \
	$(SUBSYSTEM)/connection/message.cpp \
	$(UTILS)/app.cpp \
	$(UTILS)/entity.cpp \
	$(UTILS)/timer.cpp \
	$(UTILS)/event.cpp \
	# $(UTILS)/server.cpp \
	# $(UTILS)/client.cpp \

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
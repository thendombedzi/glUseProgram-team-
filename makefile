# Source files
files = Objects/EastWall/WindowUnit.cpp Objects/EastWall/WindowWall.cpp Objects/EastWall/Verticies.cpp  shader.cpp \
    Objects/WestWall/Wall.cpp Objects/WestWall/RectangularPrism.cpp Objects/WestWall/Grids.cpp \
	Objects/WestWall/Doors.cpp \

# Compiler and flags
CXX = g++
CXXFLAGS = -g -Iglad/include -IObjects/EastWall

# Default target (Linux or Mac)
main: main.cpp glad.c
	$(CXX) $(CXXFLAGS) $(files) main.cpp glad.c -lglfw -lGLEW -ldl -lGL -pthread -o main

# Windows build target
windows:
	$(CXX) $(CXXFLAGS) $(files) main.cpp glad.c -L/mingw64/lib -I/mingw64/include -lglfw3 -lopengl32 -lglew32 -pthread -o main.exe

# Clean and Run
clean:
	rm -f *.o main main.exe

run:
	./main

all:
	make clean
	make
	make run

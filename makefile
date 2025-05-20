#add all the files that you need to the files makefile variable below as a space seperated list
files = tiny_obj_loader.cc

main: main.cpp
	g++ -std=c++17 -g shader.cpp tiny_obj_loader.cc main.cpp -lglfw -pthread -lGLEW -ldl -lGL -o main


clean:
	rm -f *.o main

run:
	./main

all:
	make clean
	make
	make run
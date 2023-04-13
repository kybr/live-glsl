.SUFFIXES:

_: main.bin

main.o: main.cpp
	c++ -std=c++14 -c main.cpp -o main.o -Iglad/include

glad.o: glad/src/glad.c
	cc -c glad/src/glad.c -o glad.o -Iglad/include

window.o: Window.h Desktop.cpp
	cc -c Desktop.cpp -o window.o -I/opt/homebrew/include -Iglad/include

main.bin: main.o glad.o window.o
	c++ -std=c++14 main.o glad.o window.o -o main.bin -L/opt/homebrew/lib -lglfw

clean:
	rm -f main.o main.bin

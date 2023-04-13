.SUFFIXES:

_: main.bin

main.o: main.cpp
	c++ -std=c++14 -g -c main.cpp -o main.o -Iglad/include -I/opt/homebrew/include

glad.o: glad/src/glad.c
	cc -g -c glad/src/glad.c -o glad.o -Iglad/include

window.o: Window.h Desktop.cpp
	c++ -std=c++14 -g -c Desktop.cpp -o window.o -I/opt/homebrew/include -Iglad/include

main.bin: main.o glad.o window.o
	c++ -std=c++14 -g main.o glad.o window.o -o main.bin -L/opt/homebrew/lib -lglfw -llo

clean:
	rm -f *.o main.bin

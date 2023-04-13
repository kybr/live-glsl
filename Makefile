.SUFFIXES:

_: bar.bin

bar.o: bar.cpp
	c++ -std=c++14 -c bar.cpp -o bar.o -Iglad/include

glad.o: glad/src/glad.c
	cc -c glad/src/glad.c -o glad.o -Iglad/include

window.o: Window.h Desktop.cpp
	cc -c Desktop.cpp -o window.o -I/opt/homebrew/include -Iglad/include

bar.bin: bar.o glad.o window.o
	c++ -std=c++14 bar.o glad.o window.o -o bar.bin -L/opt/homebrew/lib -lglfw

clean:
	rm -f bar.o bar.bin

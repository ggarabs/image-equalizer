all: compile run clean

compile: main.cpp
	g++ -o teste main.cpp -g -Og -Wall -Wno-unused -lSDL3_image -lSDL3_ttf -lSDL3 -lm

run: teste
	./teste images/input/santos.jpg

clean:
	rm -if teste
all: image-equalizer run clean

image-equalizer: main.cpp
	g++ -o teste main.cpp -g -Og -lSDL3_image -lSDL3

run: teste
	./teste ./images/santos.jpg

clean:
	rm -if teste
# makefile

all: start

start: start.cpp
	g++ -g -w -std=c++11 -o start start.cpp

clean:
	rm start

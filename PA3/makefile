# makefile

all: shell

shell: main.o
	g++ -o shell main.o -std=c++11

main.o : main.cpp
	g++ -c -g main.cpp -std=c++11

clean:
	rm -rf *.o *.csv *.txt

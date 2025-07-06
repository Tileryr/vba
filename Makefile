
all: cpu.o main.o
	g++ cpu.o main.o -o emu

main.o: main.cpp
	g++ -c main.cpp -o main.o

cpu.o: cpu.cpp
	g++ -c cpu.cpp -o cpu.o

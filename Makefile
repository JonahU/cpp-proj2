jit:
	g++ -std=c++17 -Wall -O3 run.cpp -o run.o && ./run.o examples/simple.h

debug:
	g++ -std=c++17 -Wall -g run.cpp -o run.o

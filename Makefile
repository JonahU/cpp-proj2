jit:
	g++ -std=c++17 -Wall -O3 run.cpp -o run.o && ./run.o examples/simple.h

debug:
	g++ -std=c++17 -Wall -g run.cpp -o run.o

simple.so:
	g++ -std=c++17 -Wall -shared -fPIC examples/simplest.cpp -o examples/simplest.so -lpython3.6m -lboost_python3

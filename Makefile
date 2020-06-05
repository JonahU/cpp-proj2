jit:
	g++ -std=c++17 -Wall -O3 run.cpp -o run.o && ./run.o examples/harder.h

debug:
	g++ -std=c++17 -Wall -g run.cpp -o run.o

simple.so:
	g++ -std=c++17 -Wall -shared -fPIC examples/simple.cpp -o examples/simple.so -lpython3.6m -lboost_python3

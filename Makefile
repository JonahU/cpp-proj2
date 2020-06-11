jit:
	g++ -std=c++17 -Wall -O3 run.cpp -o run.o && ./run.o examples/harder.h

debug:
	g++ -std=c++17 -Wall -g run.cpp -o run.o

simplest.so:
	g++ -std=c++17 -Wall -shared -fPIC boostpython/simplest.cpp -o examples/simplest.so -lpython3.6m -lboost_python3

clean:
	rm -f run.o

jit:
	g++ -std=c++17 -Wall -O3 run.cpp -o run.o && ./run.o examples/harder.h

debug:
	g++ -std=c++17 -Wall -g run.cpp -o run.o

hellopy:
	g++ -std=c++17 -Wall -shared -fPIC boostpython/hello.cpp -o boostpython/hello.so -lpython3.6m -lboost_python3

cppstructpy:
	g++ -std=c++17 -Wall -shared -fPIC boostpython/cppstruct.cpp -o boostpython/cppstruct.so -lpython3.6m -lboost_python3

clean:
	rm -f run.o

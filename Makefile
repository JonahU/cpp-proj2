jit:
	g++ -std=c++17 -Wall -O3 run.cpp -o run.o && ./run.o examples/harder.h

debug:
	g++ -std=c++17 -Wall -g run.cpp -o run.o

hellopy:
	g++ -std=c++17 -Wall -shared -fPIC boostpython/hello.cpp -o boostpython/hello.so -lpython3.6m -lboost_python3

aggregatepy:
	g++ -std=c++17 -Wall -shared -fPIC boostpython/aggregate.cpp -o boostpython/aggregate.so -lpython3.6m -lboost_python3

containerpy:
	g++ -std=c++17 -Wall -shared -fPIC boostpython/container.cpp -o boostpython/container.so -lpython3.6m -lboost_python3

clean:
	rm -f run.o

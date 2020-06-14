all:
	g++ -std=c++17 -Wall -O3 run.cpp -o run.o -pthread

debug:
	g++ -std=c++17 -Wall -g run.cpp -o run.o -pthread

jit:
	g++ -std=c++17 -Wall -O3 run.cpp -o run.o -pthread && ./run.o examples/simple.h

aggregatepy:
	g++ -std=c++17 -Wall -shared -fPIC boostpython/aggregate.cpp -o boostpython/aggregate.so -lpython3.6m -lboost_python3

containerpy:
	g++ -std=c++17 -Wall -shared -fPIC boostpython/container.cpp -o boostpython/container.so -lpython3.6m -lboost_python3

simplepy:
	g++ -std=c++17 -Wall -shared -fPIC examples/simple.cpp -o examples/simple.so -lpython3.6m -lboost_python3

harderpy:
	g++ -std=c++17 -Wall -shared -fPIC examples/harder.cpp -o examples/harder.so -lpython3.6m -lboost_python3

clean:
	rm -f run.o

all:
	g++ -std=c++17 -Wall -O3 run.cpp -o run.o -pthread

debug:
	g++ -std=c++17 -Wall -g run.cpp -o run.o -pthread

jit:
	g++ -std=c++17 -Wall -O3 run.cpp -o run.o -pthread && ./run.o examples/simple.h

clean:
	rm -f run.o

simple_example:
	g++ -std=c++17 -Wall -shared -fPIC examples/simple.cpp -o examples/simple.so -lpython3.6m -lboost_python3

median_example:
	g++ -std=c++17 -Wall -shared -fPIC examples/median.cpp -o examples/median.so -lpython3.6m -lboost_python3

hard_example:
	g++ -std=c++17 -Wall -shared -fPIC examples/hard.cpp -o examples/hard.so -lpython3.6m -lboost_python3

boostpython_aggregate_example:
	g++ -std=c++17 -Wall -shared -fPIC examples/boostpython/aggregate.cpp -o examples/boostpython/aggregate.so -lpython3.6m -lboost_python3

boostpython_container_example:
	g++ -std=c++17 -Wall -shared -fPIC examples/boostpython/container.cpp -o examples/boostpython/container.so -lpython3.6m -lboost_python3

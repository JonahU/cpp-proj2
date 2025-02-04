all:
	g++ -std=c++17 -Wall -O3 proj2.cpp -o proj2 -pthread

debug:
	g++ -std=c++17 -Wall -g proj2.cpp -o proj2 -pthread

jit:
	g++ -std=c++17 -Wall -O3 proj2.cpp -o proj2 -pthread && ./proj2 examples/simple.h

clean:
	rm -f proj2

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

cluster_dynamics: main.cpp
	g++ -std=c++17 -g *.cpp -o cluster_dynamics.out

mac: main.cpp
	clang -std=c++17 -g *.cpp -o cluster_dynamics.out

clean:
	rm *.out
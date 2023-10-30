cluster_dynamics: main.cpp
	g++ -std=c++17 *.cpp -o cluster_dynamics.out

clean:
	rm *.out
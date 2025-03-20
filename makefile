hw: 
	g++ -std=c++17 -c sequential.cpp -O3 -o sequential.o
	g++ -std=c++17 test.cpp sequential.o -o test -O3

clean: 
	rm test
	rm *.o
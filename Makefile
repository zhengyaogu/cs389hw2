all: Run_test

Run_test: compile_test execute_test
compile_test:
	c++ -Wall -std=c++17 -O3 -o test.o test.cc cache_lib.cc fifo_evictor.cc

execute_test:
	./test.o
	
clean:
	rm -f *.o

SS:
	g++ ConsumerTask.cpp -std=c++11 -c -Wall
	g++ Decomp.cpp -std=c++11 -c -Wall
	g++ Store.cpp  -std=c++11 -c -Wall
	g++ KeepAlive.cpp -std=c++11 -c -Wall
	g++ ConsumerTask.o   Decomp.o  KeepAlive.o  Store.o  -Wall -lpthread -lzmq -levent -g -o Demo
CC:
	g++ Client.cpp -lpthread -std=c++11 -o Client
clean:
	rm -f *.o
	rm -f Demo
	rm -f Client

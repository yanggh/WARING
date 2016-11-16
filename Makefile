install:
	g++ ConsumerTask.cpp -std=c++11 -c -Wall
	g++ decomp.cpp -std=c++11 -c -Wall
	g++ main.cpp  -std=c++11 -c -Wall
	g++ store.cpp  -std=c++11 -c -Wall
	g++  ProducerTask.cpp -std=c++11 -c -Wall
	g++   ConsumerTask.o   decomp.o main.o  ProducerTask.o  store.o  -Wall -lpthread -lzmq -levent -g -o demo
	g++  client.cpp -lpthread -std=c++11 -o client
clean:
	rm -f *.o

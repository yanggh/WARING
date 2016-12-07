SS:
	g++ ConsumerTask.cpp -std=c++11 -c -Wall
	g++ Decomp.cpp -std=c++11 -c -Wall
	g++ Store.cpp  -std=c++11 -c -Wall
	g++ Conf.cpp -std=c++11 -c -Wall  -I /usr/include/mysql/ -L /usr/lib64/mysql/ -lmysqlclient
	g++ KeepAlive.cpp -std=c++11 -c -Wall
	g++  RecvSnmp.cpp -DHAVE_CONFIG_H  -I/home/yanggh/Documents/snmp++-3.3.7   -I/home/yanggh/Documents/snmp++-3.3.7/include -std=c++11   -lpthread /home/yanggh/Documents/snmp++-3.3.7/src/.libs/libsnmp++.so -Wl,-rpath -Wl,/usr/local/lib  -c -Wall
	g++  Main.cpp -std=c++11 -c -Wall -lpthread
	g++ Conf.o  Main.o ConsumerTask.o RecvSnmp.o  Decomp.o  KeepAlive.o  Store.o  -Wall -lpthread -lzmq -levent -g -o Demo  -I/home/yanggh/Documents/snmp++-3.3.7   -I/home/yanggh/Documents/snmp++-3.3.7/include -std=c++11   -lpthread /home/yanggh/Documents/snmp++-3.3.7/src/.libs/libsnmp++.so  -L  /usr/lib64/mysql/ -lmysqlclient

CC:
	g++ Client.cpp -lpthread -std=c++11 -o Client -Wall

clean:
	rm -f *.o
	rm -f Demo
	rm -f Client

install:

uninstall:
	rm -f *.o  Demo Client

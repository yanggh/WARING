build:
	g++ ConsumerTask.cpp -std=c++11 -c -Wall -lzmq
	g++ Decomp.cpp -std=c++11 -c -Wall
	g++ Store.cpp  -std=c++11 -c -Wall
	g++ Conf.cpp -std=c++11 -c -Wall 
	g++ KeepAlive.cpp -std=c++11 -c -Wall  -I /usr/include/mysql/ -L /usr/lib64/mysql/ -lmysqlclient  -lpthread
	g++  Main.cpp -std=c++11 -c -Wall -lpthread
	g++  RecvSnmp.cpp -DHAVE_CONFIG_H  -I/home/yanggh/Documents/snmp++-3.3.7   -I/home/yanggh/Documents/snmp++-3.3.7/include -std=c++11   -lpthread  -Wl,-rpath -Wl,/usr/local/lib  -c -Wall
	g++ Conf.o  Main.o ConsumerTask.o RecvSnmp.o  Decomp.o  KeepAlive.o  Store.o  -Wall -lpthread -lzmq -levent -g -o bin/Demo  -I/home/yanggh/Documents/snmp++-3.3.7   -I/home/yanggh/Documents/snmp++-3.3.7/include -std=c++11   -lpthread /home/yanggh/Documents/snmp++-3.3.7/src/.libs/libsnmp++.so  -L  /usr/lib64/mysql/ -lmysqlclient
	g++ Client.cpp -lpthread -std=c++11 -o bin/Client -Wall
	g++ SendMod.cpp  Conf.cpp        -I /usr/local/include/modbus -L /usr/local/lib/ -lmodbus -std=c++11 -lpthread -Wall  -o  SendMod
clean:
	rm -f *.o
	rm -f bin/Demo
	rm -f bin/Client

install:
	cp -rf  bin/Demo  /usr/local/warning/bin
	cp -rf  bin/Client  /usr/local/warning/bin
	cp -rf  etc/database.conf  /usr/local/warning/etc/
	cp -rf  etc/test.conf   /usr/local/warning/etc/
	cp -rf  lib/libsnmp++.so.33    /usr/local/lib/libsnmp++.so.33
	cp -rf  lib/libzmq.so.5    /usr/local/lib/libzmq.so.5
	ldconfig	

uninstall:
	rm  /usr/local/warning/bin/Demo
	rm  /usr/local/warning/bin/Client
	rm  /usr/local/lib/libsnmp++.so.33
	rm  /usr/local/lib/libzmq.so.5


package:
	tar -czvf  warn.tar.gz  etc/  bin/  lib/ Makefile1

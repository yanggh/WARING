g++ -DHAVE_CONFIG_H  -I/home/yanggh/Documents/snmp++-3.3.7   -I/home/yanggh/Documents/snmp++-3.3.7/include $1.cpp -std=c++11   -lpthread /home/yanggh/Documents/snmp++-3.3.7/src/.libs/libsnmp++.so -Wl,-rpath -Wl,/usr/local/lib  -o  $1 -Wall 


#include <iostream>
#include <thread>
#include "Conf.h"
#include "ConsumerTask.h"
#include "KeepAlive.h"
#include "RecvSnmp.h"

using namespace std;

int main(int argc, char **argv)
{
//    string  webip(argv[1]);
//    int     webport = atoi(argv[2]);
//    string  commit(argv[3]);
//    int     trapport = atoi(argv[4]);

//    init_conf(5, 5, webip, webport, commit, trapport, "/tmp/store");

	thread  producer0(KeepAlive);
	thread  producer1(RecvUdp);
	thread  producer2(ConsumerTask);
//	thread  producer2(RecvSnmp, 162);

	producer0.join();
	producer1.join();
	producer2.join();

	return 0;
}

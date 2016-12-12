#include <iostream>
#include <thread>
#include "ConsumerTask.h"
#include "KeepAlive.h"
#include "RecvSnmp.h"
#include "Conf.h"

using namespace std;

int main(int argc, char **argv)
{
    init_conf(argv[1]);

	thread  producer0(KeepAlive);
	thread  producer1(RecvUdp);
	thread  producer2(ConsumerTask);
//	thread  producer2(RecvSnmp, 162);

	producer0.join();
	producer1.join();
	producer2.join();

	return 0;
}

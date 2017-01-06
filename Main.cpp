#include <iostream>
#include <thread>
#include "ConsumerTask.h"
#include "KeepAlive.h"
#include "RecvSnmp.h"
#include "Conf.h"
#include "SendMod.h"
#include "Daemon.h"

using namespace std;
int main(int argc, char **argv)
{
    init_conf("/usr/local/warning/etc/database.conf");

	thread  producer0(KeepAlive);
	thread  producer1(RecvUdp);
	thread  producer2(ConsumerTask);
	thread  producer3(RecvSnmp, 162);
    thread  producer4(UpdateSig);

	producer0.join();
	producer1.join();
	producer2.join();
	producer3.join();
	producer4.join();

	return 0;
}

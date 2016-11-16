#include <thread>
#include "ProducerTask.h"
#include "ConsumerTask.h"

using namespace std;

int main(int argc, char** argv)
{
	thread  producer1(ProducerTask);
	thread  consumer1(ConsumerTask);

	producer1.join();
	consumer1.join();
	return 0;
}

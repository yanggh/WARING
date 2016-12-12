#include <cstdlib>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include <unistd.h>
#include <string>
#include <cstring>
#include <zmq.hpp>
#include "Decomp.h"
#include "Conf.h"
#include "ConsumerTask.h"

#define  kItemRepositorySize  1024
#define  BUFFSIZE             256

using namespace std;

typedef  struct ItemRepository {
	char   item_buffer[kItemRepositorySize][BUFFSIZE];
	uint16_t item_len[kItemRepositorySize];
	size_t read_position;
	size_t write_position;
	size_t produced_item_counter;
	size_t consumed_item_counter;
	mutex mtx;
	mutex produced_item_counter_mtx;
	mutex consumed_item_counter_mtx;
	condition_variable repo_not_full;
	condition_variable repo_not_empty;
}ItemRepository;

static  ItemRepository  gItemRepository;

void ProduceItem(const uint8_t* item, const uint16_t len)
{
    unique_lock<mutex> pro_lock(gItemRepository.produced_item_counter_mtx);
    ++(gItemRepository.produced_item_counter);

	unique_lock<mutex> lock(gItemRepository.mtx);
	while(((gItemRepository.write_position + 1) % kItemRepositorySize)
			== gItemRepository.read_position) { // item buffer is full, just wait here.
		cout << "Producer is waiting for an empty slot...\n";
		(gItemRepository.repo_not_full).wait(lock);
	}

    memset((gItemRepository.item_buffer)[gItemRepository.write_position], 0, BUFFSIZE);
    memcpy((gItemRepository.item_buffer)[gItemRepository.write_position], item, len);
    (gItemRepository.item_len)[gItemRepository.write_position] = len;
	(gItemRepository.write_position)++;

	if (gItemRepository.write_position == kItemRepositorySize)
		gItemRepository.write_position = 0;

	(gItemRepository.repo_not_empty).notify_all();
	lock.unlock();
	
    pro_lock.unlock();
}

uint8_t *ConsumeItem(ItemRepository *ir, uint8_t* data,uint16_t *data_len)
{
	unique_lock<mutex> lock(ir->mtx);
	// item buffer is empty, just wait here.
	while(ir->write_position == ir->read_position) {
		cout << "Consumer is waiting for items...\n";
		(ir->repo_not_empty).wait(lock);
	}

    *data_len = (ir->item_len)[ir->read_position];
	memcpy((char*)data, (ir->item_buffer)[ir->read_position], (size_t)(*data_len));
	(ir->read_position)++;

	if (ir->read_position >= kItemRepositorySize)
		ir->read_position = 0;

	(ir->repo_not_full).notify_all();
	lock.unlock();

	return data;
}

void InitItemRepository(ItemRepository *ir)
{
	ir->write_position = 0;
	ir->read_position = 0;
	ir->produced_item_counter = 0;
	ir->consumed_item_counter = 0;
}


void ConsumerTask()
{
	uint8_t  *itemstr = new uint8_t[1024];
	uint16_t data_len = 0;
	InitItemRepository(&gItemRepository);

    zmq::context_t context (1);
	zmq::socket_t socket (context, ZMQ_REQ);

#if 1 
    char     webip[256];
    char     constr[256];
    bzero(constr, 256);
    bzero(webip, 256);

    get_webip(webip);
    int  webport = get_webport();
    snprintf(constr, 256, "tcp://%s:%d", webip, webport);
    printf("constr = %s\n", constr);
	socket.connect (constr);
#else
    socket.connect("tcp://192.168.34.5:5555");
#endif
	while(1) 
    {
		unique_lock<mutex> lock(gItemRepository.consumed_item_counter_mtx);
		ConsumeItem(&gItemRepository, itemstr, &data_len);
		++(gItemRepository.consumed_item_counter);
		lock.unlock();
		if(data_len > 0)
		{
			cout << "data_len = " <<  data_len << endl;

			uint8_t  jsonstr[1024];
			static int num = 0;
			uint16_t  jsonlen = 0;
			decomp(itemstr, data_len, jsonstr, &jsonlen);
			if(jsonlen != -1)
			{
				num ++;
				cout << "jsonstr = " << jsonstr << ", num = " <<  num << endl;
			}

			//work
			zmq::message_t  req(jsonlen);
			memcpy(req.data(), jsonstr, jsonlen);
			socket.send(req);

			zmq::message_t reply;
			socket.recv (&reply);
			cout << "reply.data is " << (char*)reply.data() << endl;
		}
	}

	delete []itemstr;
}

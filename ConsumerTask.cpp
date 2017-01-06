#include <cstdlib>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include <unistd.h>
#include <string>
#include <cstring>
#include "zhelpers.hpp"
#include <sstream>
#include "Decomp.h"
#include "Conf.h"
#include "ConsumerTask.h"

#define  kItemRepositorySize  1024
#define  BUFFSIZE             256

using namespace std;

typedef  struct ItemRepository {
	char   item_buffer[kItemRepositorySize][BUFFSIZE];
    uint16_t item_type[kItemRepositorySize];
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

void ProduceItem(const uint8_t* item, const uint16_t len, const uint16_t  type)
{
    unique_lock<mutex> pro_lock(gItemRepository.produced_item_counter_mtx);
    ++(gItemRepository.produced_item_counter);

	unique_lock<mutex> lock(gItemRepository.mtx);
	while(((gItemRepository.write_position + 1) % kItemRepositorySize)
			== gItemRepository.read_position) 
    {
		cout << "Producer is waiting for an empty slot...\n";
		(gItemRepository.repo_not_full).wait(lock);
	}

    memset((gItemRepository.item_buffer)[gItemRepository.write_position], 0, BUFFSIZE);
    memcpy((gItemRepository.item_buffer)[gItemRepository.write_position], item, len);
    (gItemRepository.item_type)[gItemRepository.write_position] = type;
    (gItemRepository.item_len)[gItemRepository.write_position] = len;
	(gItemRepository.write_position)++;

	if (gItemRepository.write_position == kItemRepositorySize)
		gItemRepository.write_position = 0;

	(gItemRepository.repo_not_empty).notify_all();
	lock.unlock();
	
    pro_lock.unlock();
}

static  uint8_t *ConsumeItem(ItemRepository *ir, uint8_t* data,uint16_t *data_len, uint16_t  *data_type)
{
    unique_lock<mutex> lock(ir->mtx);

    while(ir->write_position == ir->read_position) {
        cout << "Consumer is waiting for items...\n";
        (ir->repo_not_empty).wait(lock);
    }


    *data_len = (ir->item_len)[ir->read_position];
    *data_type = (ir->item_type)[ir->read_position];
    memcpy((char*)data, (ir->item_buffer)[ir->read_position], (size_t)(*data_len));
    (ir->read_position)++;

    if (ir->read_position >= kItemRepositorySize)
        ir->read_position = 0;

    (ir->repo_not_full).notify_all();
    lock.unlock();

    return data;
}

static void InitItemRepository(ItemRepository *ir)
{
    ir->write_position = 0;
    ir->read_position = 0;
    ir->produced_item_counter = 0;
    ir->consumed_item_counter = 0;
}

#if 1
void ConsumerTask()
{
    uint8_t  *itemstr = new uint8_t[1024];
    uint16_t data_len = 0;
    uint16_t data_type = 0;
    InitItemRepository(&gItemRepository);


    char     web_ip[256];
    char     constr[256];
    bzero(constr, 256);
    bzero(web_ip, 256);

    get_web_ip(web_ip);
    int  web_port = get_web_port();
    snprintf(constr, 256, "tcp://%s:%d", web_ip, web_port);

    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_REQ);
    socket.connect (constr);

    while(true) 
    {
           unique_lock<mutex> lock(gItemRepository.consumed_item_counter_mtx);
           ConsumeItem(&gItemRepository, itemstr, &data_len, &data_type);
           ++(gItemRepository.consumed_item_counter);
           lock.unlock();
           if(data_len > 0)
           {
               uint8_t  jsonstr[1024];
               static int num = 0;
               uint16_t  jsonlen = 0;
   
               bzero(jsonstr, 1024); 
               if(data_type == 0)
               {
                   decomp(itemstr, data_len, jsonstr, &jsonlen);
                   if(jsonlen != -1)
                   {
                       num ++;
                   }
               }
               else if(data_type == 1)
               {
                   memcpy(jsonstr, (char*)itemstr, strlen((char*)itemstr));
                   jsonlen = data_len;
               }
               else
               {
                   continue;
               }
   
               //work
               if(data_type == 0)
               {
   
                   zmq::message_t  req(jsonlen);
                   memcpy(req.data(), jsonstr, jsonlen);
   
                   cout << "jsonstr = " << jsonstr << endl;
                   socket.send(req);
                   zmq::message_t reply;
                   socket.recv (&reply);
                   cout << "reply.data is " << (char*)reply.data() << endl;
               }
   
           }
    }

    delete []itemstr;
}
#endif
#if 0
#define REQUEST_TIMEOUT     2500    //  msecs, (> 1000!)
#define REQUEST_RETRIES     3       //  Before we abandon

static zmq::socket_t * s_client_socket(zmq::context_t & context) 
{
    int linger = 0;
    char     web_ip[256];
    char     constr[256];
    bzero(constr, 256);
    bzero(web_ip, 256);

    get_web_ip(web_ip);
    int  web_port = get_web_port();
    snprintf(constr, 256, "tcp://%s:%d", web_ip, web_port);


    zmq::socket_t * client = new zmq::socket_t (context, ZMQ_REQ);
    client->connect (constr);

    client->setsockopt (ZMQ_LINGER, &linger, sizeof (linger));
    return client;
}

void ConsumerTask()
{
    uint8_t  *itemstr = new uint8_t[1024];
    uint16_t data_len = 0;
    uint16_t data_type = 0;
    InitItemRepository(&gItemRepository);


    zmq::context_t context (1);
    zmq::socket_t *client = s_client_socket (context);

    while(true) 
    {
        unique_lock<mutex> lock(gItemRepository.consumed_item_counter_mtx);
        ConsumeItem(&gItemRepository, itemstr, &data_len, &data_type);
        ++(gItemRepository.consumed_item_counter);
        lock.unlock();
        if(data_len > 0)
        {
            uint8_t  jsonstr[1024];
            static int num = 0;
            uint16_t  jsonlen = 0;

            bzero(jsonstr, 1024); 
            if(data_type == 0)
            {
                decomp(itemstr, data_len, jsonstr, &jsonlen);
                if(jsonlen != -1)
                {
                    num ++;
                }
            }
            else if(data_type == 1)
            {
                memcpy(jsonstr, (char*)itemstr, strlen((char*)itemstr));
                jsonlen = data_len;
            }
            else
            {
                continue;
            }

            //work
            if(data_type == 0 || data_type == 1)
            {
                stringstream request;
                request << jsonstr;
                cout << "request.str = " << request.str() << "a" << endl;
                s_send(*client, request.str());

                bool expect_reply = true;
                while (expect_reply) 
                {
//                    expect_reply = false;
                    zmq::pollitem_t items[] = { { *client, 0, ZMQ_POLLIN, 0 } };
                    zmq::poll (&items[0], 1, REQUEST_TIMEOUT);

                    cout << "If we got a reply, process it" << endl;
                    //  If we got a reply, process it
                    if (items[0].revents & ZMQ_POLLIN) {
                        break;
                        //  We got a reply from the server, must match sequence
                        string reply = s_recv (*client);
                        cout << "reply is " << reply << endl;
                        expect_reply = false;
                    }
                    else
                    {
                        cout << "W: no response from server, retrying..." << endl;
                        
                        delete client;
                        client = s_client_socket (context);
                        s_send (*client, request.str());
                    }
                }
            }
        }

        delete []itemstr;
    }
}
#endif

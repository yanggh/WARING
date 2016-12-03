#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <thread>
#include <queue>
#include <list>
#include "ConsumerTask.h"
#include "KeepAlive.h"
#include "Conf.h"
#include "Store.h"

#define WARING     0
#define KEEPALIVE  1

using namespace std;

typedef struct Node
{
    string  ip;
    int  port;
    int  flag;
    struct  sockaddr_in  sin;
}Node;

list<Node> clist;
static pthread_rwlock_t lock;

static int  sockfd = 0;

int  init_client(int num)
{
    int i = 0;
    
    Node p;

    pthread_rwlock_wrlock(&lock);
    for(i = 0; i < num; i ++)
    {
        p.ip = "192.168.34.56";
        p.port = 8882;

        bzero(&p.sin,sizeof(p.sin));  
        p.sin.sin_family=AF_INET;  
        p.sin.sin_addr.s_addr=inet_addr(p.ip.c_str());
        p.sin.sin_port=htons(p.port);

        clist.push_back(p);
    }
    pthread_rwlock_unlock(&lock);

    return 0;
}

int  create_sock()
{
    struct timeval  tv_out;
	struct sockaddr_in sin;  

    if(((sockfd = socket(AF_INET, SOCK_DGRAM, 0))< 0))
    {
        cout << "socket error" << endl;
        return -1;
    } 

	bzero(&sin,sizeof(sin));  
	sin.sin_family=AF_INET;  
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port=htons(18888);
    int sin_len = sizeof(sin);

	if(bind(sockfd, (struct sockaddr *)&sin, sin_len) < 0)
    {
        cout << "bind error" << endl;
        return -1;
    }

    tv_out.tv_sec = 1;
    tv_out.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv_out, sizeof(tv_out));

    return sockfd;
}

int CheckClient(string ip, int port)
{
    list<Node>::iterator itor;
    
    itor = clist.begin();
    pthread_rwlock_wrlock(&lock);
    while(itor != clist.end())
    {
        if((itor->ip == ip) && (itor->port == port))
        {
            itor->flag = 0;
            break;
        }
        itor ++;
    }

    pthread_rwlock_unlock(&lock);
    if(itor == clist.end())
    {
        cout << "not find dev, ip is " << ip << ", port is " << port << endl;
        return -1;
    }

    return 0;
}

int check_pack(uint8_t *str, uint16_t iLen)
{
    return 1;
}


int RecvUdp()
{
    int  iLen = 0;
    char message[1024];
    socklen_t  sin_len;
	struct sockaddr_in sin;  

    create_sock();
    int type = 0;
    while(1)
    {
        iLen = recvfrom(sockfd,  message, sizeof(message), 0, (struct sockaddr*)&sin, &sin_len);
        if(iLen > 0)
        {
            cout << "RecvUdp()" << endl;
            type = check_pack((uint8_t*)message, iLen);
            if(type == WARING) 
            {
                store((uint8_t*)message, iLen);
                sendto(sockfd, "1", 1, 0, (struct sockaddr *)&sin,  sin_len);  
                ProduceItem((uint8_t *)message, (uint16_t)iLen);
                cout << message << "  " << inet_ntoa(sin.sin_addr) << ":" << ntohs(sin.sin_port) << endl;
            }
            else if(type == KEEPALIVE)
            {
                CheckClient(inet_ntoa(sin.sin_addr), ntohs(sin.sin_port)); 
                sendto(sockfd, "1", 1, 0, (struct sockaddr *)&sin,  sin_len);  
                cout << message << "  " << inet_ntoa(sin.sin_addr) << ":" << ntohs(sin.sin_port) << endl;
            }

        }
    }
    return 0;
}

int KeepAlive()
{
    list<Node>::iterator itor;

    pthread_rwlock_rdlock(&lock);

    itor = clist.begin();
    while(itor != clist.end())
    {
        sendto(sockfd, "1212", 4, 0, (struct sockaddr*)&(itor->sin), sizeof(itor->sin));
        if(itor->flag < 10)
        {
            itor->flag ++;
        }
        else if(itor->flag == 10)
        {
            itor->flag = 0;
            cout << "keepalive" << endl;
        }
        itor++;
    }
    pthread_rwlock_unlock(&lock);
    
    return 0;
}


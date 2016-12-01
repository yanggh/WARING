#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <thread>

#include "ConsumerTask.h"
#include "Conf.h"
#include "Store.h"

#define WARING     0
#define KEEPALIVE  1

using namespace std;
typedef struct Node
{
    char  ip[16];
    int   port;
    char  flag;
    struct sockaddr_in sin;
    struct Node* next;
}Node;

typedef struct SLIST
{
    Node  *head;
    pthread_rwlock_t lock;
}SLIST;

static SLIST slist;
static int  sockfd = 0;

int  init_client(int num)
{
    int i = 0;
    
    Node *p = NULL;
    Node *q = NULL;

    pthread_rwlock_wrlock(&slist.lock);
    for(i = 0; i < num; i ++)
    {
        p = (Node*)malloc(sizeof(Node));
        memcpy(p->ip, "192.168.34.56", strlen("192.168.34.56"));
        p->port = 8882;
        p->flag = 0;

        bzero(&p->sin,sizeof(p->sin));  
        p->sin.sin_family=AF_INET;  
        p->sin.sin_addr.s_addr=inet_addr(p->ip);
        p->sin.sin_port=htons(p->port);

        if(q == NULL)
        {
            slist.head = p;
            q = slist.head;
        }
        else
        {
            q->next = p;
            q = q->next;
        }
    }
    pthread_rwlock_unlock(&slist.lock);

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
	sin.sin_port=htons(5555);
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

int CheckClient(char *ip, int port)
{
    Node *p = NULL;
    
    p = slist.head;
    pthread_rwlock_wrlock(&slist.lock);
    while(p != NULL)
    {
        if((memcmp(p->ip, ip, strlen(p->ip)) == 0) && (p->port == port))
        {
            p->flag = 0;
            break;
        }
        p = p->next;
    }

    pthread_rwlock_unlock(&slist.lock);
    if(p == NULL)
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

    int type = 0;
    while(1)
    {
        iLen = recvfrom(sockfd,  message, sizeof(message), 0, (struct sockaddr*)&sin, &sin_len);
        if(iLen > 0)
        {
            type = check_pack((uint8_t*)message, iLen);
            if(type == WARING) 
            {
                store((uint8_t*)message, iLen);
                sendto(sockfd, "1", 1, 0, (struct sockaddr *)&sin,  sin_len);  
                ProduceItem((uint8_t *)message, (uint16_t)iLen);
            }
            else if(type == KEEPALIVE)
            {
                CheckClient(inet_ntoa(sin.sin_addr), ntohs(sin.sin_port)); 
            }

            cout << message << "  " << inet_ntoa(sin.sin_addr) << ":" << ntohs(sin.sin_port) << endl;
        }
    }
    return 0;
}

int KeepAlive()
{
    Node *p = NULL;

    pthread_rwlock_rdlock(&slist.lock);

    p = slist.head;
    while(p != NULL)
    {
        sendto(sockfd, "1212", 4, 0, (struct sockaddr*)&(p->sin), sizeof(p->sin));
        if(p->flag < 10)
        {
            p->flag ++;
        }
        else if(p->flag == 10)
        {
            p->flag = 0;
            cout << "keepalive" << endl;
        }

        p = p->next;
    }
    pthread_rwlock_unlock(&slist.lock);
    
    return 0;
}

int main(int argc, char **argv)
{
    int  num = 10;

    if(argc == 2)
    {
        num = atoi(argv[1]);
    }
    else if(argc > 2)
    {
        cout << "please use " << argv[0] << " listnum " << endl;
        return -1;
    }


    pthread_rwlock_init(&slist.lock, NULL);

    init_client(num);
    sockfd = create_sock();

	thread  producer0(KeepAlive);
    thread  producer1(RecvUdp);
    thread  producer2(ConsumerTask);

    producer0.join();
    producer1.join();
    producer2.join();

    pthread_rwlock_destroy(&slist.lock);
    return 0;
}

#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <thread>
#include <queue>
#include <list>
#include <sys/time.h>
#include <signal.h>
#include "ConsumerTask.h"
#include "KeepAlive.h"
#include "Conf.h"
#include "Store.h"

#define WARING     0
#define KEEPALIVE  1
#define TIMEOUT    2

using namespace std;

typedef struct TT{
    uint8_t   year_h;
    uint8_t   year_l;
    uint8_t   mon;
    uint8_t   day;
    uint8_t   hh;
    uint8_t   mm;
    uint8_t   ss;
}TT;

typedef struct ALIVE
{
    uint8_t type;
    uint8_t flen;
    uint8_t son_sys;
    TT  tt;
}ALIVE;

typedef struct Node
{
    uint8_t  son_sys;
    uint8_t  stop;
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
        p.son_sys = 1;
        p.stop = 1;
        p.ip = "192.168.34.56";
        p.port = 88889;
        p.flag = 0;

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
        if((itor->ip == ip))
        {
            itor->flag = 2 * TIMEOUT;
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

int check_pack(const uint8_t *str, const uint16_t iLen, short *fnum)
{
    if((str[0] == 0xFF && str[1] == 0x7E) || (str[0] == 0x7E && str[1] == 0xFF))
    {
        *fnum = *(short*)(str+2);
        return WARING;     
    }
    else if(str[0] == 0xFF)
    {
        return KEEPALIVE;
    }

    return  0;
}

int getalive(ALIVE *alive)
{
    time_t tt = time(NULL);
    struct tm *t = localtime(&tt);

    alive->type = 0xaa;
    alive->flen = sizeof(ALIVE);
    alive->son_sys = rand() % 0x0a;

    alive->tt.year_h = (t->tm_year + 1900) / 100;
    alive->tt.year_l = (t->tm_year + 1900) % 100;
    alive->tt.mon = t->tm_mon;
    alive->tt.day = t->tm_mday;
    alive->tt.hh = t->tm_hour;
    alive->tt.mm = t->tm_min;
    alive->tt.ss = t->tm_sec; 

    return 0;
}

int RecvUdp()
{
    int  iLen = 0;
    char message[1024];
    socklen_t  sin_len;
	struct sockaddr_in sin;  

    create_sock();
    int type = 0;
    short fnum = 0;
    while(1)
    {
        iLen = recvfrom(sockfd,  message, sizeof(message), 0, (struct sockaddr*)&sin, &sin_len);
        if(iLen > 0)
        {
            type = check_pack((uint8_t*)message, iLen, &fnum);
            if(type == WARING) 
            {
                store((uint8_t*)message, iLen);
                sendto(sockfd, (uint8_t*)&fnum, 2, 0, (struct sockaddr *)&sin,  sin_len);  
                ProduceItem((uint8_t *)message, (uint16_t)iLen);
                cout << message << ", WARING" << inet_ntoa(sin.sin_addr) << ":" << ntohs(sin.sin_port) << endl;
            }
            else if(type == KEEPALIVE)
            {
                CheckClient(inet_ntoa(sin.sin_addr), ntohs(sin.sin_port)); 
                cout << message << ", KEEPALIVE" << inet_ntoa(sin.sin_addr) << ":" << ntohs(sin.sin_port) << endl;
            }
        }
    }
    return 0;
}

//timer work
void  alive(int signo)
{
    list<Node>::iterator itor;
    pthread_rwlock_rdlock(&lock);
    itor = clist.begin();

    //cout << "alive start" << endl;
    ALIVE alive;
    getalive(&alive);

    while(itor != clist.end())
    {
        if(itor->flag > 0)
        {
            itor->flag --;
        }
        else if(itor->flag == 0)
        {
            itor->flag = 2 * TIMEOUT;
            sendto(sockfd, (void*)&alive, sizeof(ALIVE), 0, (struct sockaddr*)&(itor->sin), sizeof(itor->sin));
            cout << "keepalive time out" << endl;
            ProduceItem((uint8_t *)"message", 7);
        }

        itor++;
    }
    pthread_rwlock_unlock(&lock);
}

int KeepAlive()
{
    struct itimerval tick;

    init_client(2);
    signal(SIGALRM,  alive);
    memset(&tick, 0, sizeof(tick));

    tick.it_value.tv_sec = TIMEOUT;
    tick.it_value.tv_usec = 0;

    tick.it_interval.tv_sec = TIMEOUT;
    tick.it_interval.tv_usec = 0;

    if(setitimer(ITIMER_REAL, &tick, NULL) < 0)
        cout << "Set timer failed!" << endl;

    while(1)
    {
        pause();
    }
    return 0;
}

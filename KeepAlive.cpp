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
#include <mysql.h>
#include <zmq.hpp>
#include <pthread.h>
#include "ConsumerTask.h"
#include "KeepAlive.h"
#include "Conf.h"
#include "Store.h"

#define  WARING     0
#define  KEEPALIVE  1
#define  KEEP_ALIVE_STR   "{ type: \"0xFFFF\", fnum: \"0\", flen: \"0\", son_sys: \"%d\", stop: \"%d\", eng: \"0\", node:\"0\", bug: \"0\", time: \"0\", res1: \"%s\", res2: \"%d\", res3: \"%d\", check: \"0\"}"
#define  MYSQL_STR   "select  trsts.server_ip,trsts.server_port,trs.STATION_ORDER_NO,tr.SUBSYSTEM_CODE  from TBL_RELATION_STATION_TO_SUBSYSTEM trsts,TBL_RESMANAGE_STATIONINFO trs,TBL_RESMANAGE_SUBSYSTEMINFO tr where trsts.STATIONID=trs.STATIONID AND trsts.SUBSYSTEMID=tr.SUBSYSTEMID limit 1"

#define  CONNECT    1
#define  DISCON     0

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
    int  timeout;
    struct  sockaddr_in  sin;
}Node;

enum  NUM
{
    IPSUB = 0,
    PORTSUB = 1,
    STOPSUB = 2,
    SONSYSSUB = 3
};

static list<Node> clist;
static int  sockfd = 0;
static pthread_mutex_t lock;

static  int   keep_json_str(const int  son_sys, const int stop, const char* ip, const int port, const int flag)
{
    char buf[256];
    bzero(buf, 256);
    int iLen = snprintf(buf, 256, KEEP_ALIVE_STR,  son_sys, stop, ip, port, flag);
    if(iLen > 0)
    {
//        cout << "keep_json_str state update: iLen = " << iLen << ", buf = " << buf << endl;
        ProduceItem((uint8_t *)buf, iLen, KEEPALIVE);
    }
    else
    {
        cout << "keep_json_str error " << endl; 
    }

    return 0;
}

static int  create_sock()
{
    struct timeval  tv_out;
	struct sockaddr_in sin;  

    if(((sockfd = socket(AF_INET, SOCK_DGRAM, 0))< 0))
    {
        cout << "socket error" << endl;
        return -1;
    } 

    int sport = get_sport();

	bzero(&sin,sizeof(sin));  
	sin.sin_family=AF_INET;  
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port=htons(sport);
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

static  int CheckClient(string ip,  int port)
{
    int   timeout = get_keepalive();

    list<Node>::iterator itor;

    pthread_mutex_lock(&lock);
    itor = clist.begin();
    while(itor != clist.end())
    {
        if((itor->ip == ip))
        {
            itor->timeout = timeout;

            if(itor->flag == DISCON)
            {
                itor->flag = CONNECT;
                keep_json_str(itor->son_sys, itor->stop, itor->ip.c_str(), itor->port, itor->flag);
            }
            break;
        }
        itor ++;
    }
    pthread_mutex_unlock(&lock);

    return 0;
}

static  int check_pack(const uint8_t *str, const uint16_t iLen, short *fnum)
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

    return  -1;
}

static int getalive(ALIVE *alive)
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

static  void  alive(int signo)
{
    ALIVE alive;
    char buf[256];
    getalive(&alive);
   
    list<Node>::iterator itor;
    
    pthread_mutex_lock(&lock);
    itor = clist.begin();
    while(itor != clist.end())
    {
        if(itor->timeout > 0)
        {
            itor->timeout --;
        }
        else if(itor->timeout == 0)
        {
            if(itor->flag == CONNECT)
            {
                itor->flag = DISCON;
                bzero(buf, 256);
                keep_json_str(itor->son_sys, itor->stop, itor->ip.c_str(), itor->port, itor->flag);
            }
        }
        sendto(sockfd, (void*)&alive, sizeof(ALIVE), 0, (struct sockaddr*)&(itor->sin), sizeof(itor->sin));
        itor++;
    }
    pthread_mutex_unlock(&lock);
}

static  int  init_client(void)
{
    MYSQL mysql;
    MYSQL_RES *res;
    MYSQL_ROW row;

    int  ret = 0;
    int  timeout = get_keepalive();
    pthread_mutex_lock(&lock);
    if(!clist.empty())
    {
        clist.clear();
    }
    pthread_mutex_unlock(&lock);

    mysql_init(&mysql);

    char mysqlip[256];
    char username[256];
    char password[256];
    char database[256];

    get_mysql_ip(mysqlip);
    get_username(username);
    get_password(password);
    get_database(database);

    if(!mysql_real_connect(&mysql,mysqlip, username, password, database, 0, NULL, 0))
    {
        cout << "无法连接到数据库，错误原因是:" << mysql_error(&mysql) << endl;
        return  -1;
    }

    unsigned  int t=mysql_real_query(&mysql, MYSQL_STR,(unsigned int)strlen(MYSQL_STR));

    if(t)
    {
        cout << "查询数据库失败" << mysql_error(&mysql) << endl;
        ret = -1;
    }
    else 
    {
        Node p; 
        res = mysql_store_result(&mysql);//返回查询的全部结果集
        if(res != NULL)
        {
            while((row=mysql_fetch_row(res)) > 0)
            {
                //mysql_fetch_row取结果集的下一行
                p.son_sys = atoi(row[SONSYSSUB]);
                p.stop = atoi(row[STOPSUB]);
#if 0
                p.ip = row[IPSUB];
                p.port = atoi(row[PORTSUB]);
#else
                p.ip = "192.168.34.28";
                p.port = 18887;
#endif
                p.flag = CONNECT;
                p.timeout = timeout;

                bzero(&p.sin,sizeof(p.sin));  
                p.sin.sin_family=AF_INET;  
                p.sin.sin_addr.s_addr=inet_addr(p.ip.c_str());
                p.sin.sin_port=htons(p.port);

                pthread_mutex_lock(&lock);
                clist.push_back(p);
                pthread_mutex_unlock(&lock);
            }
        }
        else
        {
            ret = -1;
        }
    }

    mysql_free_result(res);
    mysql_close(&mysql);
    return ret;
}

int RecvUdp()
{
    int  iLen = 0;
    int  ret = 0;
    char message[1024];
    socklen_t  sin_len;
	struct sockaddr_in sin;  

    int type = 0;
    short fnum = 0;
    
    ret = create_sock();
    if(ret == -1)
    {
        cout << "create_sock error " << endl;
        return -1;
    }

    while(true)
    {
        iLen = recvfrom(sockfd,  message, sizeof(message), 0, (struct sockaddr*)&sin, &sin_len);
        if(iLen > 0)
        {
            type = check_pack((uint8_t*)message, iLen, &fnum);
            if(type == WARING) 
            {
                store((uint8_t*)message, iLen);
                sendto(sockfd, (uint8_t*)&fnum, 2, 0, (struct sockaddr *)&sin,  sin_len);  
                ProduceItem((uint8_t *)message, (uint16_t)iLen, WARING);
                cout << "WARING: " << inet_ntoa(sin.sin_addr) << ":" << ntohs(sin.sin_port) << endl;
            }
            else if(type == KEEPALIVE)
            {
                CheckClient(inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));
                cout << "KEEPALIVE: " << inet_ntoa(sin.sin_addr) << ":" << ntohs(sin.sin_port) << endl;
            }
        }
    }
    return 0;
}

//timer work
int KeepAlive()
{
    struct itimerval tick;

    pthread_mutex_init(&lock, NULL);
    int ret = init_client();
    if(ret == -1)
    {
        cout << "not find client ip and port" << endl;
    }

    signal(SIGALRM,  alive);
    memset(&tick, 0, sizeof(tick));

    tick.it_value.tv_sec = 1;
    tick.it_value.tv_usec = 0;

    tick.it_interval.tv_sec = 1;
    tick.it_interval.tv_usec = 0;

    if(setitimer(ITIMER_REAL, &tick, NULL) < 0)
        cout << "Set timer failed!" << endl;

    while(true)
    {
        pause();
    }
    return 0;
}

int  UpdateSig()
{
    char     update_ip[256];
    char     constr[256];
    bzero(constr, 256);
    bzero(update_ip, 256);

    get_update_ip(update_ip);
    int update_port = get_update_port();
    snprintf(constr, 256, "tcp://*:%d", update_port);

    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_REP);
	socket.bind (constr);

    while (true) 
    {
        zmq::message_t request;

        // 等待客户端请求
        socket.recv (&request);
        
        if(memcmp(request.data(), "update", request.size() == 0))
        {
            int  ret = init_client();
            if(ret == -1)
            {
                cout << "update error" << endl;
            }
            
        }
        else
        {
            cout << "error: request.size() = " << request.size() << endl;
        }

        // 应答World
        zmq::message_t reply (5);
        memcpy ((void *) reply.data (), "World", 5);
        socket.send (reply);
    }

    return 0;
}


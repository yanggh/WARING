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
#include "ConsumerTask.h"
#include "KeepAlive.h"
#include "Conf.h"
#include "Store.h"

#define WARING     0
#define KEEPALIVE  1

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
static int  sockfd = 0;

#if 0
int  init_client()
{
    MYSQL mysql;
    MYSQL_RES *res;
    MYSQL_ROW row;

    mysql_init(&mysql);

    char mysqlip[256];
    char username[256];
    char password[256];
    char database[256];

    get_mysqlip(mysqlip);
    get_username(username);
    get_password(password);
    get_database(database);

    if(!mysql_real_connect(&mysql,mysqlip, username, password, database, 0, NULL, 0))
    {
        cout << "无法连接到数据库，错误原因是:" << mysql_error(&mysql) << endl;
    }

    char sqlcmd[256];
    sprintf(sqlcmd,"%s","select * from friends");
    unsigned  int t=mysql_real_query(&mysql,sqlcmd,(unsigned int)strlen(sqlcmd));

    if(t)
    {
        cout << "查询数据库失败" << mysql_error(&mysql) << endl;
    }
    else 
    {
        res = mysql_store_result(&mysql);//返回查询的全部结果集
        
        while((row=mysql_fetch_row(res)) > 0)
        {
            //mysql_fetch_row取结果集的下一行
            for(t = 0; t < mysql_num_fields(res); t++)
            {
                //结果集的列的数量
                cout << row[t] << "\t";
            }
            clist.push_back(p);
        }
    }

    mysql_free_result(res);
    mysql_close(&mysql);
    return 0;
}
#endif

int  init_client(int num)
{
    int i = 0;
    
    Node p;

    if(!clist.empty())
    {
        clist.clear();
    }

    for(i = 0; i < num; i ++)
    {
        p.son_sys = 1;
        p.stop = 1;
        p.ip = "192.168.34.28";
        p.port = 18887;
        p.flag = 0;

        bzero(&p.sin,sizeof(p.sin));  
        p.sin.sin_family=AF_INET;  
        p.sin.sin_addr.s_addr=inet_addr(p.ip.c_str());
        p.sin.sin_port=htons(p.port);

        clist.push_back(p);
    }

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

#if 1 
    int sport = get_sport();
#else
    int sport = 18888;
#endif

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

int CheckClient(string ip, int port)
{
    int   flag = 0;
    list<Node>::iterator itor;
    
    itor = clist.begin();
#if 1 
    int   timeout = get_keepalive();
#else
    int   timeout = 2;
#endif

    while(itor != clist.end())
    {
        if((itor->ip == ip))
        {
            flag = 1;
            itor->flag = 2 * timeout;
            break;
        }
        itor ++;
    }

    if(flag == 0)
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
void  alive(int signo)
{
    list<Node>::iterator itor;
    itor = clist.begin();

    //cout << "alive start" << endl;
    ALIVE alive;
    char buf[256];
    getalive(&alive);
    
    int  flag = 0;
#if 1 
    int  timeout = get_keepalive();
#else
    int  timeout = 2;
#endif
    while(itor != clist.end())
    {
        flag = 0;
        if(itor->flag > 0)
        {
            itor->flag --;
        }
        else if(itor->flag == 0)
        {
            itor->flag = 2 * timeout;
            flag = 1;
        }

        sendto(sockfd, (void*)&alive, sizeof(ALIVE), 0, (struct sockaddr*)&(itor->sin), sizeof(itor->sin));
        if(flag == 1)
        {
            cout << "keepalive time out" << endl;
            bzero(buf, 256);
            snprintf(buf, 256, "{ son_sys:  \"%d\", stop: \"%d\", ip :\" %s\", port: \"%d\", flag : -1 }", itor->son_sys, itor->stop, itor->ip.c_str(), itor->port);
            printf("buf = %s\n", buf);
        }
        //ProduceItem(buf, strlen(buf));
        itor++;
    }
}

int KeepAlive()
{
    struct itimerval tick;

    init_client(1);
    signal(SIGALRM,  alive);
    memset(&tick, 0, sizeof(tick));

#if 1 
    int timeout = get_keepalive();
#else
    int timeout = 2;
#endif
    tick.it_value.tv_sec = timeout;
    tick.it_value.tv_usec = 0;

    tick.it_interval.tv_sec = timeout;
    tick.it_interval.tv_usec = 0;

    if(setitimer(ITIMER_REAL, &tick, NULL) < 0)
        cout << "Set timer failed!" << endl;

    while(1)
    {
        pause();
    }
    return 0;
}


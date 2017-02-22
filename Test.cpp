#include <thread>
#include <list>
#include <unistd.h>
#include <iostream>
#include <strings.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <syslog.h>

#pragma  pack (1)
using namespace std;

static  int  timeout = 0;

typedef struct TT{
    uint8_t   year_h;
    uint8_t   year_l;
    uint8_t   mon;
    uint8_t   day;
    uint8_t   hh;
    uint8_t   mm;
    uint8_t   ss;
}TT;

typedef struct SEGMENT
{
    uint16_t  type;
    uint16_t  fnum;
    uint8_t   flen;
    uint8_t   son_sys;
    uint8_t   stop;
    uint8_t   eng;
    uint8_t   node;

    uint16_t   bug;

    TT         tt;
    uint16_t  res1;
    uint16_t  res2;
    uint16_t  res3;

    uint16_t  check;
}SEGMENT;

typedef struct SHAKE
{
    uint8_t   type;
    uint8_t   len;
    uint8_t   son_sys;

    TT         tt;
}SHAKE;

static  int socket_descriptor = 0;
static  list<SEGMENT> warnlist;

void produce(string ip, int port)
{
    int i = 0;
    SEGMENT seg;
    static uint16_t num = 0;

    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family=AF_INET;
    address.sin_addr.s_addr=inet_addr(ip.c_str());
    address.sin_port=htons(port);

    printf("ip.c_str = %s, port = %d\n", ip.c_str(), port);

    seg.type = 0xff7e;
    seg.fnum = (num++) % 0xffff;
    seg.flen = sizeof(SEGMENT);

    list<SEGMENT>::iterator itor;
    while(1)
    {
        for(i = 0; i < 50; i++)
        {
            //create node
TODO:
            time_t tt = time(NULL);
            struct tm *t = localtime(&tt);

            seg.son_sys = rand() % 2 ? 1 : 6;
            seg.stop = rand() % 2 + 1;
            seg.eng = 1;
            seg.node = rand() % 2 + 1;
            //    	seg.bug = seg.son_sys == 1 ? (rand() % 119 + (rand() % 2 == 0 ? 0 : 4096)) : ((seg.son_sys == 6 ? (rand() % 16 +  4215) : 4215) - (rand() % 2 == 0 ?  0 : 4096));
            seg.bug = seg.son_sys == 1 ? (rand() % 119 + 4096) : (seg.son_sys == 6 ? (rand() % 2 +  4215) : 4215);
            seg.tt.year_h = (t->tm_year + 1900) / 100;
            seg.tt.year_l = (t->tm_year + 1900) % 100;
            seg.tt.mon = t->tm_mon + 1;
            seg.tt.day = t->tm_mday;
            seg.tt.hh = t->tm_hour;
            seg.tt.mm = t->tm_min;
            seg.tt.ss = t->tm_sec; 

            seg.res1 = (seg.son_sys == 8 ? rand() % 6 + 1 : ( seg.son_sys == 2 ? rand() % 5 + 1 : (seg.son_sys == 3 ? rand() % 3 + 1 : (seg.son_sys == 4 ? rand() % 3 : 0))));
            seg.res2 = (seg.son_sys == 8 ? rand() % 100 : 0);
            seg.res3 = (seg.son_sys == 8 ? rand() % 100 : 0);

            seg.check = 0;

            for(itor = warnlist.begin(); itor != warnlist.end(); itor++)
            {
                if((itor->son_sys == seg.son_sys) 
                        && (itor->stop == seg.stop)
                        && (itor->eng == seg.eng) 
                        && (itor->node == seg.node)
                        && (itor->bug == seg.bug))
                {
                    cout << "TODO" << endl;
                    goto TODO;
                }
            }

            warnlist.push_back(seg);
        }

        itor = warnlist.begin();
        for(i = 0; i < 50; i++)
        {
            time_t tt = time(NULL);
            struct tm *t = localtime(&tt);

            seg.son_sys = itor->son_sys;
            seg.stop = itor->stop;
            seg.eng = itor->eng;
            seg.node = itor->node;
            seg.bug = itor->bug - 4096;

            seg.tt.year_h = (t->tm_year + 1900) / 100;
            seg.tt.year_l = (t->tm_year + 1900) % 100;
            seg.tt.mon = t->tm_mon + 1;
            seg.tt.day = t->tm_mday;
            seg.tt.hh = t->tm_hour;
            seg.tt.mm = t->tm_min;
            seg.tt.ss = t->tm_sec; 

            seg.res1 = itor->res1;
            seg.res2 = itor->res2;
            seg.res3 = itor->res3;

            seg.check = 0;

            warnlist.push_back(seg);
            itor ++;
        }


        int num = 0;
        itor = warnlist.begin();
        while(itor != warnlist.end())
        {
            num ++;
            usleep(timeout * 100);
            sendto(socket_descriptor, (uint8_t*)&(*itor), sizeof(SEGMENT), 0,  (struct sockaddr *)&address,  sizeof(address));
#ifdef DEBUG
            printf("son_sys: %d  ", itor->son_sys);
            printf("stop: %02d  ", itor->stop);
            printf("eng: %02d  ", itor->eng);
            printf("node: %02d  ", itor->node);
            printf("bug: %04d  ", itor->bug);
            printf("num = %d\n", num);
#endif
            itor ++;
        }
    
        warnlist.clear();
    }
}

void  consum(int  port)
{
    int ret;
    int sin_len;
    char  message[256];

    struct sockaddr_in sin;

    bzero(&sin,sizeof(sin));
    sin.sin_family=AF_INET;
    sin.sin_addr.s_addr=htonl(INADDR_ANY);
    sin.sin_port=htons(port);
    sin_len=sizeof(sin);

    socket_descriptor=socket(AF_INET,SOCK_DGRAM,0);
    bind(socket_descriptor,(struct sockaddr *)&sin,sizeof(sin));

    struct timeval tv_out;
    tv_out.tv_sec = 1;
    tv_out.tv_usec = 0;
    setsockopt(socket_descriptor, SOL_SOCKET, SO_RCVTIMEO, &tv_out, sizeof(tv_out));

    SHAKE    *q = NULL;
    SHAKE    shake;

    while(true)
    {
        ret = recvfrom(socket_descriptor,  message,   256, 0, (struct sockaddr *)&sin, (socklen_t*)&sin_len);
        message[ret] = '\0';

        if(ret > 0)
        {
            if(ret == sizeof(SHAKE))
            {
                syslog(LOG_DEBUG, "keep alive msg");
                q = (SHAKE*)message;

                shake.type = 0xff;
                shake.len = sizeof(SHAKE);
                shake.son_sys = q->son_sys;

                shake.tt.year_h = q->tt.year_h;
                shake.tt.year_l = q->tt.year_l;
                shake.tt.mon = q->tt.mon;
                shake.tt.day = q->tt.day;
                shake.tt.hh = q->tt.hh;
                shake.tt.mm = q->tt.mm;
                shake.tt.ss = q->tt.ss;

                sendto(socket_descriptor, (uint8_t*)&shake, sizeof(SHAKE), 0,  (struct sockaddr *)&sin,  sizeof(sin));
            }
            else
            {
                syslog(LOG_INFO, "recv error num %02x", ret);
            }
        }
    }
    close(socket_descriptor);
}

int main(int argc, char **argv)
{
    FILE* fp = fopen(argv[1], "r");
    if(NULL == fp)
    {
        return -1;
    }

    char buff[1024] = {0};
    char buf1[1024] = {0};
    char buf2[1024] = {0};
    int  send_port = 0;
    int  recv_port = 0;
    string ip_addr;

    while(fgets(buff, 1024, fp) != NULL)
    {
        bzero(buf1, 1024);
        bzero(buf2, 1024);
        sscanf(buff, "%s  %s", buf1, buf2);
        if(memcmp(buf1, "send_port", strlen(buf1)) == 0)
        {
            send_port = atoi(buf2); 
        }
        else  if(memcmp(buf1, "recv_port", strlen(buf1)) == 0)
        {
            recv_port = atoi(buf2); 
        }
        else  if(memcmp(buf1, "time_sec", strlen(buf1)) == 0)
        {
            timeout = atoi(buf2); 
        }
        else  if(memcmp(buf1, "ip_addr", strlen(buf1)) == 0)
        {
            ip_addr = string(buf2); 
        }
        else
        {
            syslog(LOG_INFO, "error");
        }

        bzero(buff, 1024);
    }
    fclose(fp);
    fp = NULL;

    syslog(LOG_ERR, "send_port = %d, recv_port = %d, timeout = %d, ip_addr = %s.", send_port, recv_port, timeout, ip_addr.c_str());

    thread produce1(produce, ip_addr, send_port);
    thread consum1(consum, recv_port);

    produce1.join();
    consum1.join();
    
    return 0;
}

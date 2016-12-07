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

#pragma  pack (1)
using namespace std;

static  const int sport=18888;
static  const int rport=18887;

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
static  list<SEGMENT> seglist;

void produce(string ip)
{
    SEGMENT seg;
    static uint16_t num = 0;

    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family=AF_INET;
    address.sin_addr.s_addr=inet_addr(ip.c_str());
    address.sin_port=htons(sport);

    list<SEGMENT>::iterator itor;
    while(1)
    {
        itor = seglist.begin();
        while(itor != seglist.end())
        {
            if(itor->check == 1)
            {
                //sendto(socket_descriptor, (uint8_t*)&itor, sizeof(SEGMENT), 0,  (struct sockaddr *)&address,  sizeof(address));
                break;
            }
            itor ++;
        }

        usleep(5000);
        //create node
        time_t tt = time(NULL);
        struct tm *t = localtime(&tt);
       
        seg.type = 0xff7e;
        seg.fnum = (num++) % 0xffff;
        seg.flen = sizeof(SEGMENT);
        seg.son_sys = rand() % 0x0a;
        seg.stop = rand() % 0x20;
        seg.eng = rand() % 0x10;
        seg.node = rand() % 167 + 0x100;

        seg.tt.year_h = (t->tm_year + 1900) / 100;
        seg.tt.year_l = (t->tm_year + 1900) % 100;
        seg.tt.mon = t->tm_mon;
        seg.tt.day = t->tm_mday;
        seg.tt.hh = t->tm_hour;
        seg.tt.mm = t->tm_min;
        seg.tt.ss = t->tm_sec; 

       seg.res1 = (seg.son_sys == 8 ? rand() % 6 + 1 : ( seg.son_sys == 2 ? rand() % 5 + 1 : (seg.son_sys == 3 ? rand() % 3 + 1 : (seg.son_sys == 4 ? rand() % 3 : 0))));
        seg.res2 = (seg.son_sys == 8 ? rand() % 100 : 0);
        seg.res3 = (seg.son_sys == 8 ? rand() % 100 : 0);

        seg.check = 0;
  
        sendto(socket_descriptor, (uint8_t*)&seg, sizeof(SEGMENT), 0,  (struct sockaddr *)&address,  sizeof(address));
        seglist.push_back(seg);
    }
}

void  consum()
{
    int ret;
    int sin_len;
    char  message[256];

    struct sockaddr_in sin;

    bzero(&sin,sizeof(sin));
    sin.sin_family=AF_INET;
    sin.sin_addr.s_addr=htonl(INADDR_ANY);
    sin.sin_port=htons(rport);
    sin_len=sizeof(sin);

    socket_descriptor=socket(AF_INET,SOCK_DGRAM,0);
    bind(socket_descriptor,(struct sockaddr *)&sin,sizeof(sin));

    struct timeval tv_out;
    tv_out.tv_sec = 1;
    tv_out.tv_usec = 0;
    setsockopt(socket_descriptor, SOL_SOCKET, SO_RCVTIMEO, &tv_out, sizeof(tv_out));

    SHAKE    *q = NULL;
    SHAKE    shake;

    list<SEGMENT>::iterator itor;

    while(1)
    {
        ret = recvfrom(socket_descriptor,  message,   256, 0, (struct sockaddr *)&sin, (socklen_t*)&sin_len);
        message[ret] = '\0';

        int  flag = 0;
        if(ret > 0)
        {
            if(ret == 2)
            {
                itor = seglist.begin();
                while(itor != seglist.end())
                {
                    if(itor->fnum == atoi(message))
                    {
                        seglist.erase(itor);
                        flag = 1;
                        break;
                    }
                    itor ++;
                }

                if(flag == 1)
                {
                    cout << "not find send, num = " << atoi(message)  << endl;
                }
            }
            else
            {
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
        }
    }
    close(socket_descriptor);
}

int main(int argc, char **argv)
{
    
    thread produce1(produce, argv[1]);
    thread consum1(consum);

    produce1.join();
    consum1.join();
    
    return 0;
}

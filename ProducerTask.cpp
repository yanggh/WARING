#include <iostream>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <endian.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "store.h"
#include "ProducerTask.h"
#include "ConsumerTask.h"

using namespace std;
static const int sport = 18887;
static const int rport = 18888;

#define MAXIMUM_LENGTH 10
#define BUFES 20
#define SIZE 10

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

    uint8_t   year_h;
    uint8_t   year_l;
    uint8_t   mon;
    uint8_t   day;
    uint8_t   hh;
    uint8_t   mm;
    uint8_t   ss;

    uint16_t  res1;
    uint16_t  res2;
    uint16_t  res3;

    uint16_t  check;
}SEGMENT;

int check_pack(uint8_t *data, uint16_t data_len)
{
    if(data_len == sizeof(SEGMENT))
    {
        return -1; 
    }

    return 0;
}

void  ProducerTask()
{  

	int sin_len;

	int socket_descriptor;  
	struct sockaddr_in sin;  

	bzero(&sin,sizeof(sin));  
	sin.sin_family=AF_INET;  
    sin.sin_addr.s_addr=inet_addr("192.168.34.66");
	sin.sin_port=htons(rport);
	sin_len=sizeof(sin);  

    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family=AF_INET;
    address.sin_addr.s_addr=inet_addr("192.168.34.66");
    address.sin_port=htons(sport);

	socket_descriptor=socket(AF_INET,SOCK_DGRAM,0);  
	bind(socket_descriptor,(struct sockaddr *)&sin,sizeof(sin));  

    struct timeval tv_out;
    tv_out.tv_sec = 1;
    tv_out.tv_usec = 0;
    setsockopt(socket_descriptor, SOL_SOCKET, SO_RCVTIMEO, &tv_out, sizeof(tv_out));

    int  num = 0;
    char buf = 0xfe;
    char message[1024];
    int  iLen = 0;
    while(1)  
    {  
	    iLen = recvfrom(socket_descriptor,message, 1024, 0, (struct sockaddr *)&sin,(socklen_t*)&sin_len);	
	    if(iLen > 0)
	    {
		    check_pack((uint8_t*)message, (uint16_t)iLen);
		    store((uint8_t*)message, iLen);
		    sendto(socket_descriptor, &buf, 1, 0, (struct sockaddr *)&address,  sizeof(address));  
		    ProduceItem((uint8_t *)message, (uint16_t)iLen);
		    num ++; 
		    if(strncmp(message,"stop",4) == 0)  
		    {  
			    printf("Sender has told me to end the connection\n");  
			    break;  
		    }
	    }
	    else
	    {
		    cout << "iLen is " << iLen << endl;
	    }
    } 

	close(socket_descriptor);  
}  

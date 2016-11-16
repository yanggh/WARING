#include <string.h>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <event.h>
#include "ProducerTask.h"
#include "ConsumerTask.h"

using namespace std;

static struct event_base* base;

void onRead(int iCliFd, short iEvent, void *arg)
{
    uint16_t iLen = 0;
    uint8_t buf[1024];
    bool bCloseClient = true;

    iLen = recv(iCliFd, buf, 1024, 0);

    if (iLen <= 0) 
    {
        cout << "Client Close" << endl;
    }
    else
    {
        buf[iLen] = 0;

        cout << "iLen is ---------"<< iLen << endl;
//      ProduceItem((uint8_t*)buf, iLen);
        bCloseClient = false;
    }

    if(bCloseClient)
    {
        struct event *pEvRead = (struct event*)arg;
        event_del(pEvRead);
        delete pEvRead;

        close(iCliFd);
    }
}

void onAccept(int iSvrFd, short iEvent, void *arg)
{
    int iCliFd;
    struct sockaddr_in sCliAddr;

    socklen_t iSinSize = sizeof(sCliAddr);
    iCliFd = accept(iSvrFd, (struct sockaddr*)&sCliAddr, &iSinSize);
    if(iCliFd != -1)
    {
        unsigned short port = sCliAddr.sin_port;
        const char* ip = inet_ntoa(sCliAddr.sin_addr);
        cout << "\nClient[" << ip << ":" << port << "] is connected" << endl;

        struct event *pEvRead = new event;
        event_set(pEvRead, iCliFd, EV_READ | EV_PERSIST, onRead, pEvRead);
        event_base_set(base, pEvRead);
        event_add(pEvRead, NULL);
    }
}

void  ProducerTask()
{
    int iSvrFd;
    struct sockaddr_in sSvrAddr;

    memset(&sSvrAddr, 0, sizeof(sSvrAddr));
    
    sSvrAddr.sin_family = AF_INET;
    sSvrAddr.sin_addr.s_addr = inet_addr("192.168.34.66");
    sSvrAddr.sin_port = htons(8888);

    iSvrFd = socket(AF_INET, SOCK_STREAM, 0);
    bind(iSvrFd, (struct sockaddr*)&sSvrAddr, sizeof(sSvrAddr));
    
    listen(iSvrFd, 10);
    cout << "Tcp server start successful, listening on port " << 8888 << endl;

    base = event_base_new();

    struct event evListen;
    event_set(&evListen, iSvrFd, EV_READ | EV_PERSIST, onAccept, NULL);
    event_base_set(base, &evListen);
    event_add(&evListen, NULL);
    event_base_dispatch(base);
}


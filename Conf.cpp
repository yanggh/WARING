#include <iostream>
#include <thread>
#include <unistd.h>
#include "Conf.h"

typedef struct ST_CONF
{
    int  keepalive;
    int  sport;
    string  webip;
    int  webport;
    string dir;
    pthread_rwlock_t  lock;
}ST_CONF;

static  ST_CONF  conffile;

int get_keepalive()
{
    return conffile.keepalive;
}

int set_keepalive(const int keepalive)
{
    conffile.keepalive = keepalive;
    return 0;
}

int comp_keepalive(const int keepalive)
{
    return (conffile.keepalive == keepalive ? 0 : -1);
}

int get_sport()
{
    return conffile.sport;
}

int set_sport(const int sport)
{
    conffile.sport = sport;
    return 0;
}

int comp_sport(const int sport)
{
    return (conffile.sport == sport ? 0 : -1);
}

int get_webip(string ip)
{
    ip = conffile.webip;
    return 0;
}

int set_webip(const string ip)
{
    conffile.webip = ip;
    return 0;
}

int comp_webip(const string ip)
{
    return ip == conffile.webip ? 0 : -1;
}

int get_webport()
{
    return conffile.webport;
}

int set_webport(const int webport)
{
    conffile.webport = webport;
    return 0;
}

int comp_webport(const int webport)
{
    return conffile.webport == webport ? 0 : -1;
}

int get_dir(string dir)
{
    dir = conffile.dir;
    return 0;
}

int set_dir(const string dir)
{
    conffile.dir = dir;
    return 0;
}

int comp_dir(const string dir)
{
    return dir == conffile.dir ? 0 : -1;
}

int init_conf(int keepalive = 5, int sport = 5, string webip = "127.0.0.1", int webport = 100, string dir = "/tmp/store")
{
    pthread_rwlock_init(&conffile.lock, NULL);

    pthread_rwlock_wrlock(&conffile.lock);
    set_keepalive(keepalive);
    set_sport(sport);
    set_webip(webip);
    set_webport(webport);
    set_dir(dir);
    pthread_rwlock_unlock(&conffile.lock);
    return 0;
}

int reload_conf(int keepalive = 5, int sport = 5, string webip = "127.0.0.1", int webport = 100, string dir = "/tmp/store")
{
    if(comp_keepalive(keepalive) != 0)
    {
        pthread_rwlock_wrlock(&conffile.lock);
        set_keepalive(keepalive);
        pthread_rwlock_unlock(&conffile.lock);
    } 

    if(comp_sport(sport) != 0)
    {
        pthread_rwlock_wrlock(&conffile.lock);
        set_sport(sport);
        pthread_rwlock_unlock(&conffile.lock);
    }

    if(comp_webip(webip) != 0)
    {   
        pthread_rwlock_wrlock(&conffile.lock);
        set_webip(webip);
        pthread_rwlock_unlock(&conffile.lock);
    }

    if(comp_webport(webport) != 0)
    {
        pthread_rwlock_wrlock(&conffile.lock);
        set_webport(webport);
        pthread_rwlock_unlock(&conffile.lock);
    }

    if(comp_dir(dir) != 0)
    {
        pthread_rwlock_wrlock(&conffile.lock);
        set_dir(dir);
        pthread_rwlock_unlock(&conffile.lock);
    }

    return 0;
}

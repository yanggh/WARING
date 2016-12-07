#include <iostream>
#include <thread>
#include <unistd.h>
#include <mysql.h>
#include <string.h>
#include "Conf.h"

typedef struct ST_CONF
{
    int  keepalive;
    int  sport;
    string  webip;
    int  webport;
    string  commit;
    int  trapport;
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

int  get_commit(string  commit)
{
    commit = conffile.commit;
    return 0;
}

int set_commit(const string commit)
{
    conffile.commit = commit;
    return 0;
}

int comp_commit(const string commit)
{
    return (conffile.commit == commit ? 0 : -1);
}

int  get_trapport(int trapport)
{
    trapport = conffile.trapport;
    return 0;
}

int set_trapport(const int trapport)
{
    conffile.trapport = trapport;
    return 0;
}

int comp_trapport(const int trapport)
{
    return (conffile.trapport == trapport ? 0 : -1);
}

int init_conf(int keepalive, int sport, string webip, int webport, string commit, int trapport, string dir)
{
    pthread_rwlock_init(&conffile.lock, NULL);
    pthread_rwlock_wrlock(&conffile.lock);
    set_keepalive(keepalive);
    set_sport(sport);
    set_webip(webip);
    set_webport(webport);
    set_dir(dir);
    set_commit(commit);
    set_trapport(trapport);
    pthread_rwlock_unlock(&conffile.lock);

    //cout << "init conf file" << endl;
    return 0;
}

int get_database()
{
    MYSQL mysql;
    MYSQL_RES *res;
    MYSQL_ROW row;

    //char* server_groups[]={"embedded", "server", "this_program_server",(char*)NULL};
    //mysql_library_init(0,NULL,server_groups);
    mysql_init(&mysql);

    if(!mysql_real_connect(&mysql,"192.168.1.139","root","root","db_record",0,NULL,0))
    {
        cout << "无法连接到数据库，错误原因是:" << mysql_error(&mysql) << endl;
    }

    char sqlcmd[1024];
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
        }
    }

    mysql_free_result(res);
    mysql_close(&mysql);
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

    //cout << "reload conf file " << endl;
    return 0;
}

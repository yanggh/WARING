#include <iostream>
#include <thread>
#include <unistd.h>
#include <string.h>
#include "Conf.h"

#define  KEEPALIVE  "keepalive"
#define  LOGDIR     "log-dir"
#define  LISTENPORT "listen-port"
#define  WEBIP      "webip"
#define  WEBPORT    "webport"
#define  COMMIT     "commit"
#define  SNMPPORT   "trapport"
#define  USERNAME   "username"
#define  PASSWORD   "password"
#define  MYSQLIP    "mysqlip"
#define  DATABASE   "database"

typedef struct ST_CONF
{
    int  keepalive;
    char dir[256];
    int  sport;
    char webip[256];
    int  webport;
    char commit[256];
    int  trapport;
    char username[256]; 
    char password[256];
    char mysqlip[256];
    char database[256];
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

int get_webip(char* ip)
{
    memcpy(ip, conffile.webip, strlen(conffile.webip));
    return 0;
}

int set_webip(const char* ip)
{
    memcpy(conffile.webip, ip, strlen(ip));
    return 0;
}

int comp_webip(const char* ip)
{
    return memcmp(ip, conffile.webip, strlen(conffile.webip)) == 0 ? 0 : -1;
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

int get_dir(char* dir)
{
    memcpy(dir, conffile.dir, strlen(conffile.dir));
    return 0;
}

int set_dir(const char* dir)
{
    memcpy(conffile.dir, dir, strlen(dir));
    return 0;
}

int comp_dir(const char* dir)
{
    return memcmp(dir, conffile.dir, strlen(conffile.dir)) == 0 ? 0 : -1;
}

int  get_commit(char*  commit)
{
    memcpy(commit, conffile.commit, strlen(conffile.commit));
    return 0;
}

int set_commit(const char* commit)
{
    bzero(conffile.commit, 256);
    memcpy(conffile.commit, commit, strlen(commit));
    return 0;
}

int comp_commit(const char* commit)
{
    return (memcmp(conffile.commit, commit, strlen(commit)) == 0 ? 0 : -1);
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

int  get_username(char* username)
{
    memcpy(username, conffile.username, strlen(conffile.username));
    return 0;
}

int  set_username(const char* username)
{
    memcpy(conffile.username, username, strlen(conffile.username));
    return 0;
}

int  get_password(char* password)
{
    memcpy(password, conffile.password, strlen(conffile.password));
    return 0;
}

int  set_password(const char* password)
{
    bzero(conffile.password, 256);
    memcpy(conffile.password, password, strlen(password));
    return 0;
}

int  get_mysqlip(char *mysqlip)
{
    memcpy(mysqlip, conffile.mysqlip, strlen(conffile.mysqlip));
    return 0;
}

int  set_mysqlip(const char* mysqlip)
{
    bzero(conffile.mysqlip, 256);
    memcpy(conffile.mysqlip, mysqlip, strlen(mysqlip));
    return 0;
}

int  get_database(char* database)
{
    memcpy(database, conffile.database, strlen(conffile.database));
    return 0;
}

int  set_database(const char* database)
{
    bzero(conffile.database, 256);
    memcpy(conffile.database, database, strlen(database));
    return 0;
}

int init_conf(const char* conffile)
{
    char  buf1[256];
    char  buf2[256];
    char  buff[256];

    FILE  *fp = fopen(conffile,  "r");

    while(fgets(buff, 256, fp) != NULL)
    {
        sscanf(buff, "%s %s", buf1, buf2);

        if(memcmp(buf1, KEEPALIVE, strlen(KEEPALIVE)) == 0)
        {
            set_keepalive(atoi(buf2));        
        }
        else if(memcmp(buf1, LOGDIR, strlen(LOGDIR)) == 0)
        {
            set_dir(buf2); 
        }
        else if(memcmp(buf1, LISTENPORT, strlen(LISTENPORT)) == 0)
        {
            set_sport(atoi(buf2)); 
        }
        else if(memcmp(buf1, WEBIP, strlen(WEBIP)) == 0)
        {
            set_webip(buf2); 
        }
        else if(memcmp(buf1, WEBPORT, strlen(WEBPORT)) == 0)
        {
            set_webport(atoi(buf2));
        }
        else if(memcmp(buf1, COMMIT, strlen(COMMIT)) == 0)
        {
            set_commit(buf2); 
        }
        else if(memcmp(buf1, SNMPPORT, strlen(SNMPPORT)) == 0)
        {
            set_trapport(atoi(buf2));
        }
        else if(memcmp(buf1, USERNAME, strlen(USERNAME)) == 0)
        {
            set_username(buf2);        
        }
        else if(memcmp(buf1, PASSWORD, strlen(PASSWORD)) == 0)
        {
            set_password(buf2); 
        }
        else if(memcmp(buf1, MYSQLIP, strlen(MYSQLIP)) == 0)
        {
            set_mysqlip(buf2);
        }
        else if(memcmp(buf1, DATABASE, strlen(DATABASE)) == 0)
        {
            set_database(buf2);
        }
        else
        {
            cout << "Not find buff" << endl;
        }

        bzero(buf1, 256);
        bzero(buf2, 256);
        bzero(buff, 256);  
    } 
    fclose(fp);

    return 0;
}

#include <iostream>
#include <thread>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include "Conf.h"

#define  KEEPALIVE  "keepalive"
#define  LOGDIR     "log-dir"
#define  LISTENPORT "listen-port"
#define  WEBIP      "web-ip"
#define  WEBPORT    "web-port"
#define  UPDATEIP      "update-ip"
#define  UPDATEPORT    "update-port"
#define  COMMIT     "commit"
#define  SNMPPORT   "trap-port"
#define  USERNAME   "username"
#define  PASSWORD   "password"
#define  MYSQLIP    "mysql-ip"
#define  DATABASE   "database"
#define  MODBUS_IP   "modbus-ip"
#define  MODBUS_PORT   "modbus-port"



typedef struct ST_CONF
{
    int  keepalive;
    char dir[256];
    int  sport;
    char web_ip[256];
    int  web_port;
    char update_ip[256];
    int  update_port;
    char commit[256];
    int  trap_port;
    char username[256]; 
    char password[256];
    char mysql_ip[256];
    char database[256];
    char modbus_ip[256];
    int  modbus_port;
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

int get_update_ip(char* ip)
{
    memcpy(ip, conffile.update_ip, strlen(conffile.update_ip));
    return 0;
}

int set_update_ip(const char* ip)
{
    memcpy(conffile.update_ip, ip, strlen(ip));
    return 0;
}

int comp_update_ip(const char* ip)
{
    return memcmp(ip, conffile.update_ip, strlen(conffile.update_ip)) == 0 ? 0 : -1;
}

int get_update_port()
{
    return conffile.update_port;
}

int set_update_port(const int update_port)
{
    conffile.update_port = update_port;
    return 0;
}

int comp_update_port(const int update_port)
{
    return conffile.update_port == update_port ? 0 : -1;
}

int get_web_ip(char* ip)
{
    memcpy(ip, conffile.web_ip, strlen(conffile.web_ip));
    return 0;
}

int set_web_ip(const char* ip)
{
    memcpy(conffile.web_ip, ip, strlen(ip));
    return 0;
}

int comp_web_ip(const char* ip)
{
    return memcmp(ip, conffile.web_ip, strlen(conffile.web_ip)) == 0 ? 0 : -1;
}

int get_web_port()
{
    return conffile.web_port;
}

int set_web_port(const int web_port)
{
    conffile.web_port = web_port;
    return 0;
}

int comp_web_port(const int web_port)
{
    return conffile.web_port == web_port ? 0 : -1;
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

int  get_trap_port()
{
    return conffile.trap_port;
}

int set_trap_port(const int trap_port)
{
    conffile.trap_port = trap_port;
    return 0;
}

int comp_trap_port(const int trap_port)
{
    return (conffile.trap_port == trap_port ? 0 : -1);
}

int  get_username(char* username)
{
    memcpy(username, conffile.username, strlen(conffile.username));
    return 0;
}

int  set_username(const char* username)
{
    memcpy(conffile.username, username, strlen(username));
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

int  get_mysql_ip(char *mysql_ip)
{
    memcpy(mysql_ip, conffile.mysql_ip, strlen(conffile.mysql_ip));
    return 0;
}

int  set_mysql_ip(const char* mysql_ip)
{
    bzero(conffile.mysql_ip, 256);
    memcpy(conffile.mysql_ip, mysql_ip, strlen(mysql_ip));
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

int  get_modbus_ip(char* modbus_ip)
{
    memcpy(modbus_ip, conffile.modbus_ip, strlen(conffile.modbus_ip));
    return 0;
}

int  set_modbus_ip(const char* modbus_ip)
{
    bzero(conffile.modbus_ip, 256);
    memcpy(conffile.modbus_ip, modbus_ip, strlen(modbus_ip));
    return 0;
}

int  get_modbus_port()
{
    return conffile.modbus_port;
}

int  set_modbus_port(const int modbus_port)
{
    conffile.modbus_port = modbus_port;
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
        bzero(buf1, 256);
        bzero(buf2, 256);
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
        else if(memcmp(buf1, UPDATEIP, strlen(UPDATEIP)) == 0)
        {
            set_update_ip(buf2); 
        }
        else if(memcmp(buf1, UPDATEPORT, strlen(UPDATEPORT)) == 0)
        {
            set_update_port(atoi(buf2));
        }
        else if(memcmp(buf1, WEBIP, strlen(WEBIP)) == 0)
        {
            set_web_ip(buf2); 
        }
        else if(memcmp(buf1, WEBPORT, strlen(WEBPORT)) == 0)
        {
            set_web_port(atoi(buf2));
        }
        else if(memcmp(buf1, COMMIT, strlen(COMMIT)) == 0)
        {
            set_commit(buf2); 
        }
        else if(memcmp(buf1, SNMPPORT, strlen(SNMPPORT)) == 0)
        {
            set_trap_port(atoi(buf2));
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
            set_mysql_ip(buf2);
        }
        else if(memcmp(buf1, DATABASE, strlen(DATABASE)) == 0)
        {
            set_database(buf2);
        }
        else if(memcmp(buf1, MODBUS_IP, strlen(MODBUS_IP)) == 0)
        {
            set_modbus_ip(buf2);
        }
        else if(memcmp(buf1, MODBUS_PORT, strlen(MODBUS_PORT)) == 0)
        {
            set_modbus_port(atoi(buf2));
        }
        else
        {
            syslog(LOG_INFO, "buf1 find %s, buf2 find %s", buf1, buf2);
        }

        bzero(buff, 256);  
    } 
    fclose(fp);

    return 0;
}




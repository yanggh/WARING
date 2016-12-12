#ifndef __CONF__H
#define __CONF__H
using namespace std;

int get_keepalive();
int set_keepalive(const int keepalive);
int comp_keepalive(const int keepalive);

int get_sport();
int set_sport(const int sport);
int comp_sport(const int sport);

int get_webip(char* ip);
int set_webip(const char* ip);
int comp_webip(const char* ip);

int get_webport();
int set_webport(const int webport);
int comp_webport(const int webport);

int get_dir(char* dir);
int set_dir(const char* dir);
int comp_dir(const char* dir);

int  get_commit(char*  commit);
int  set_commit(const char* commit);
int  comp_commit(const char* commit);

int  get_trapport(int trapport);
int  set_trapport(const int trapport);
int  comp_trapport(const int trapport);

int  set_username(const char* username);
int  set_password(const char* password);
int  set_mysqlip(const char* mysqlip);
int  set_database(const char* database);

int  get_username(char* username);
int  get_password(char* password);
int  get_mysqlip(char* mysqlip);
int  get_database(char* database);

int init_conf(const char* conffile);
int init_list();
#endif

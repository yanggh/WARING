#ifndef __CONF__H
#define __CONF__H
using namespace std;

int get_keepalive();
int set_keepalive(const int keepalive);
int comp_keepalive(const int keepalive);

int get_sport();
int set_sport(const int sport);
int comp_sport(const int sport);

int get_update_ip(char* ip);
int set_update_ip(const char* ip);
int comp_update_ip(const char* ip);

int get_update_port();
int set_update_port(const int update_port);
int comp_update_port(const int update_port);

int get_web_ip(char* ip);
int set_web_ip(const char* ip);
int comp_web_ip(const char* ip);

int get_web_port();
int set_web_port(const int web_port);
int comp_web_port(const int web_port);

int get_dir(char* dir);
int set_dir(const char* dir);
int comp_dir(const char* dir);

int  get_commit(char*  commit);
int  set_commit(const char* commit);
int  comp_commit(const char* commit);

int  get_trap_prt(int trap_prt);
int  set_trap_prt(const int trap_prt);
int  comp_trap_prt(const int trap_prt);

int  set_username(const char* username);
int  set_password(const char* password);
int  set_mysql_ip(const char* mysql_ip);
int  set_database(const char* database);

int  get_username(char* username);
int  get_password(char* password);
int  get_mysql_ip(char* mysql_ip);
int  get_database(char* database);

int init_conf(const char* conffile);
int init_list();

#endif

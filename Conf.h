#ifndef __CONF__H
#define __CONF__H
using namespace std;

int get_keepalive();
int set_keepalive(const int keepalive);
int comp_keepalive(const int keepalive);

int get_sport();
int set_sport(const int sport);
int comp_sport(const int sport);

int get_webip(string ip);
int set_webip(const string ip);
int comp_webip(const string ip);

int get_webport();
int set_webport(const int webport);
int comp_webport(const int webport);

int get_dir(string dir);
int set_dir(const string dir);
int comp_dir(const string dir);

int init_conf(int keepalive, int sport, string webip, int webport, string commit, int trapport, string dir);
int reload_conf(int keepalive, int sport, string webip, int webport);


int get_commit(string  commit);
int set_commit(const string commit);
int comp_commit(const string commit);

#endif

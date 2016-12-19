#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <iostream>
#include "Store.h"
#include "Conf.h"

using namespace std;
int  store(const  uint8_t *data, const uint16_t data_len)
{
    static  FILE*    fp = NULL;
    static  uint16_t num = 0;

    char filename[1024] = {0};
    char dir[1024] = {0};
    char cmd[1024] = {0};
    get_dir(dir);

    if(access(dir, F_OK) != 0)
    {
        cout << dir << endl;
        snprintf(cmd, 1024, "mkdir -p %s", dir);
        int status = system(cmd);
        if(-1 == status)
        {
            cout << "system error!" << endl;
        }
        else
        {
            if(WIFEXITED(status))
            {
                if (0 == WEXITSTATUS(status))
                {
                    cout << "run shell script success!" << endl;
                }
                else
                {
                    cout << "run shell script fail, script exit code: " << WEXITSTATUS(status) << endl;
                }
                return -1;
            }
            else
            {
                cout << "exit status = [" << WEXITSTATUS(status) << "]" << endl;
            }
        }
    }

    if(fp == NULL)
    {
        time_t  t = time(NULL);
        struct  tm *tt = localtime(&t);
  
        bzero(filename, 1024);
        snprintf(filename, 1024, "%s/%04d%02d%02d%02d%02d%02d", dir, tt->tm_year + 1900, tt->tm_mon, tt->tm_mday, tt->tm_hour, tt->tm_min, tt->tm_sec);
        fp = fopen(filename, "a+");
        if(NULL == fp)
        {
            cout << "fopen " << filename << " error" << endl;
            return -1;
        }
    }

    if(num < 100000)
    {
        num++; 
        fwrite(data, 1, data_len, fp);
        fflush(fp);
    }
    else
    {
        fclose(fp);
        fp = NULL;

        time_t  t = time(NULL);
        struct  tm *tt = localtime(&t);

        bzero(filename, 1024);
        snprintf(filename, 1024, "%s/%04d%02d%02d%02d%02d%02d", dir, tt->tm_year + 1900, tt->tm_mon, tt->tm_mday, tt->tm_hour, tt->tm_min, tt->tm_sec);
        fp = fopen(filename, "a+");
        if(NULL == fp)
        {
            cout << "fopen " << filename << " error" << endl;
            return -1;
        }

        num = 0;
    }
    
    return 0;
}

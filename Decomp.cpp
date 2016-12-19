#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <iostream>
#include "Decomp.h"

#pragma  pack (1)
using namespace std;

#define  WAR_JSON_STR   "{ type: \"%d\", fnum: \"%d\", flen: \"%d\", son_sys: \"%d\", stop: \"%d\", eng: \"%d\", node:\"%d\", bug: \"%d\", time: \"%02d%02d-%02d-%02d %02d:%02d:%02d\", res1: \"%d\", res2: \"%d\", res3: \"%d\", check: \"%d\"}"
#define  SHAKE_JSON_STR   "{ type:\"%d\", len: \"%d\", son_sys: \"%d\", time: \"%02d%02d-%02d-%02d  %02d:%02d:%02d\"}"

typedef struct TT{
    uint8_t   year_h;
    uint8_t   year_l;
    uint8_t   mon;
    uint8_t   day;
    uint8_t   hh;
    uint8_t   mm;
    uint8_t   ss;
}TT;

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

    TT         tt;

    uint16_t  res1;
    uint16_t  res2;
    uint16_t  res3;

    uint16_t  check;
}SEGMENT;

typedef struct SHAKE
{
    uint8_t   type;
    uint8_t   len;
    uint8_t   son_sys;

    TT         tt;
}SHAKE;

int  decomp(const uint8_t *input, const uint16_t inlen, uint8_t *output, uint16_t *outlen)
{
    uint8_t  stype1;
    uint8_t  stype2;
    stype1 = *(uint8_t*)input;
    stype2 = *(uint8_t*)(input+1);

    if((stype1 == 0x7e && stype2 == 0xff) || (stype1 == 0xff && stype2 == 0x7e))
    {
        SEGMENT *p = NULL;
        p = (struct SEGMENT*)input;

        if(p->son_sys == 0x02)
        {
            if(p->res1 == 1)
            {
                p->res1 = 0;
            }
            else if(p->res1 == 2 || p->res1 == 3)
            {
                p->res1 = 1;
            }
            else if(p->res1 == 4 || p->res1 == 5)
            {
                p->res1 = 2;
            }
        }
        else if(p->son_sys == 0x03)
        {
            if(p->res1 == 3)
            {
                p->res1 = 0;
            }
            else if(p->res1 == 2)
            {
                p->res1 = 1; 
            }
            else if(p->res1 == 1)
            {
                p->res1 = 2;
            }
        }

        *outlen = sprintf((char*)output, WAR_JSON_STR, p->type, p->fnum, p->flen, p->son_sys, p->stop,  p->eng,  p->node,  p->bug,  p->tt.year_h,  p->tt.year_l,  p->tt.mon, p->tt.day,  p->tt.hh,  p->tt.mm,   p->tt.ss,   p->res1, p->res2, p->res3, p->check);
    }
    else if(stype1 == 0xaa || stype1 == 0xff)
    {
        SHAKE  *p = NULL;
        p = (struct SHAKE*)input;
        *outlen = sprintf((char*)output, SHAKE_JSON_STR, p->type, p->len, p->son_sys, p->tt.year_h,  p->tt.year_l,  p->tt.mon, p->tt.day,  p->tt.hh,  p->tt.mm,   p->tt.ss);
        
    }
    else
    {
        *outlen = -1;
    }

    return 0;
}

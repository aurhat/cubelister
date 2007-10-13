
#include <stdio.h>
#include <string.h>
#include "cube_tools.h"

void putint(ucharbuf &p, int n)
{
    if (n<128 && n>-127) p.put(n);
    else if (n<0x8000 && n>=-0x8000)
    {
        p.put(0x80);
        p.put(n);
        p.put(n>>8);
    }
    else
    {
        p.put(0x81);
        p.put(n);
        p.put(n>>8);
        p.put(n>>16);
        p.put(n>>24);
    }
}

int getint(ucharbuf &p)
{
    int c = (char)p.get();
    if (c==-128)
    {
        int n = p.get();
        n |= char(p.get())<<8;
        return n;
    }
    else if (c==-127)
    {
        int n = p.get();
        n |= p.get()<<8;
        n |= p.get()<<16;
        return n|(p.get()<<24);
    }
    else return c;
}

void putstring(const char *t, ucharbuf &p)
{
    while (*t) putint(p,*t++);
    putint(p,0);
}

void getstring(char *text, ucharbuf &p, int len)
{
    char *t=text;
    do
    {
        if (t>=&text[len])
        {
            text[len-1]=0;
            return;
        }
        if (!p.remaining())
        {
            *t=0;
            return;
        }
        *t=getint(p);
    }
    while (*t++);
}

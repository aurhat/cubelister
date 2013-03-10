
#include "Csl.h"

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

int getuint(ucharbuf &p)
{
    int n = p.get();
    if (n & 0x80)
    {
        n += (p.get() << 7) - 0x80;
        if (n & (1<<14)) n += (p.get() << 14) - (1<<14);
        if (n & (1<<21)) n += (p.get() << 21) - (1<<21);
        if (n & (1<<28)) n |= 0xF0000000;
    }
    return n;
}

void putstring(const char *t, ucharbuf &p)
{
    while (*t) putint(p,*t++);
    putint(p, 0);
}

int getstring(char *text, ucharbuf &p, int len)
{
    int l = 0;
    char *t = text;
    do
    {
        if (t>=&text[len])
        {
            text[len-1] = 0;
            return l;
        }
        if (!p.remaining())
        {
            *t=0;
            return l;
        }
        if ((*t = getint(p)))
            l++;
    }
    while (*t++);
    return l;
}

///////////////////////// character conversion ///////////////

int iscubeprint(int c)
{
    static const char flags[256] =
    {
        0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
    };
    return flags[c];
}

int cube2uni(int c)
{
    static const int conv[256] =
    {
        0, 192, 193, 194, 195, 196, 197, 198, 199, 9, 10, 11, 12, 13, 200, 201,
        202, 203, 204, 205, 206, 207, 209, 210, 211, 212, 213, 214, 216, 217, 218, 219,
        32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
        48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
        64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
        80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
        96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
        112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 220,
        221, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237,
        238, 239, 241, 242, 243, 244, 245, 246, 248, 249, 250, 251, 252, 253, 255, 0x104,
        0x105, 0x106, 0x107, 0x10C, 0x10D, 0x10E, 0x10F, 0x118, 0x119, 0x11A, 0x11B, 0x11E, 0x11F, 0x130, 0x131, 0x141,
        0x142, 0x143, 0x144, 0x147, 0x148, 0x150, 0x151, 0x152, 0x153, 0x158, 0x159, 0x15A, 0x15B, 0x15E, 0x15F, 0x160,
        0x161, 0x164, 0x165, 0x16E, 0x16F, 0x170, 0x171, 0x178, 0x179, 0x17A, 0x17B, 0x17C, 0x17D, 0x17E, 0x404, 0x411,
        0x413, 0x414, 0x416, 0x417, 0x418, 0x419, 0x41B, 0x41F, 0x423, 0x424, 0x426, 0x427, 0x428, 0x429, 0x42A, 0x42B,
        0x42C, 0x42D, 0x42E, 0x42F, 0x431, 0x432, 0x433, 0x434, 0x436, 0x437, 0x438, 0x439, 0x43A, 0x43B, 0x43C, 0x43D,
        0x43F, 0x442, 0x444, 0x446, 0x447, 0x448, 0x449, 0x44A, 0x44B, 0x44C, 0x44D, 0x44E, 0x44F, 0x454, 0x490, 0x491
    };
    return conv[c];
}

int encodeutf8(uchar *dstbuf, int dstlen, const uchar *srcbuf, int srclen, int *carry)
{
    uchar *dst = dstbuf, *dstend = &dstbuf[dstlen];
    const uchar *src = srcbuf, *srcend = &srcbuf[srclen];
    if(src < srcend && dst < dstend) do
    {
        int uni = cube2uni(*src);
        if(uni <= 0x7F)
        {
            if(dst >= dstend) goto done;
            const uchar *end = min(srcend, &src[dstend-dst]);
            do
            {
                *dst++ = uni;
                if(++src >= end) goto done;
                uni = cube2uni(*src);
            }
            while(uni <= 0x7F);
        }
        if(uni <= 0x7FF) { if(dst + 2 > dstend) goto done; *dst++ = 0xC0 | (uni>>6); goto uni2; }
        else if(uni <= 0xFFFF) { if(dst + 3 > dstend) goto done; *dst++ = 0xE0 | (uni>>12); goto uni3; }
        else if(uni <= 0x1FFFFF) { if(dst + 4 > dstend) goto done; *dst++ = 0xF0 | (uni>>18); goto uni4; }
        else if(uni <= 0x3FFFFFF) { if(dst + 5 > dstend) goto done; *dst++ = 0xF8 | (uni>>24); goto uni5; }
        else if(uni <= 0x7FFFFFFF) { if(dst + 6 > dstend) goto done; *dst++ = 0xFC | (uni>>30); goto uni6; }
        else goto uni1;
    uni6: *dst++ = 0x80 | ((uni>>24)&0x3F);
    uni5: *dst++ = 0x80 | ((uni>>18)&0x3F);
    uni4: *dst++ = 0x80 | ((uni>>12)&0x3F);
    uni3: *dst++ = 0x80 | ((uni>>6)&0x3F);
    uni2: *dst++ = 0x80 | (uni&0x3F);
    uni1:;
    }
    while(++src < srcend);

done:
    if(carry) *carry += src - srcbuf;
    return dst - dstbuf;
}

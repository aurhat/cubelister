// generic useful stuff for any C++ program

#ifndef CUBE_TOOLS_H
#define CUBE_TOOLS_H

#include <stdlib.h>

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;

#ifdef _DEBUG
#ifdef __GNUC__
#define ASSERT(c) if(!(c)) { asm("int $3"); }
#else
#define ASSERT(c) if(!(c)) { __asm int 3 }
#endif
#else
#define ASSERT(c) if(c) {}
#endif

#ifdef swap
#undef swap
#endif
/*template<class T>
static inline void swap(T &a, T &b)
{
    T t = a;
    a = b;
    b = t;
}*/
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
template<class T>
static inline T max(T a, T b)
{
    return a > b ? a : b;
}
template<class T>
static inline T min(T a, T b)
{
    return a < b ? a : b;
}

#define clamp(a, b, c) (max(b, min(a, c)))

#define loop(v, m) for(int v = 0; v<int(m); v++)
#define loopi(m) loop(i, m)
#define loopj(m) loop(j, m)
#define loopk(m) loop(k, m)
#define loopl(m) loop(l, m)

#ifndef WIN32
#define __cdecl
#endif

#define _MAXDEFSTR 260

#define loopv(v)    if(false) {} else for(int i = 0; i<(v).length(); i++)
#define loopvj(v)   if(false) {} else for(int j = 0; j<(v).length(); j++)
#define loopvk(v)   if(false) {} else for(int k = 0; k<(v).length(); k++)
#define loopvrev(v) if(false) {} else for(int i = (v).length()-1; i>=0; i--)

template <class T>
struct databuf
{
    enum
    {
        OVERREAD  = 1<<0,
        OVERWROTE = 1<<1
    };

    T *buf;
    int len, maxlen;
    uchar flags;

    template<class U>
    databuf(T *buf, U maxlen) : buf(buf), len(0), maxlen((int)maxlen), flags(0) {}

    T* at(const int pos)
    {
        return &buf[pos];
    }

    const T &get()
    {
        static T overreadval;
        if (len<maxlen) return buf[len++];
        flags |= OVERREAD;
        return overreadval;
    }

    databuf subbuf(int sz)
    {
        sz = min(sz, maxlen-len);
        len += sz;
        return databuf(&buf[len-sz], sz);
    }

    void put(const T &val)
    {
        if (len<maxlen) buf[len++] = val;
        else flags |= OVERWROTE;
    }

    void put(const T *vals, int numvals)
    {
        if (maxlen-len<numvals) flags |= OVERWROTE;
        memcpy(&buf[len], vals, min(maxlen-len, numvals)*sizeof(T));
        len += min(maxlen-len, numvals);
    }

    int get(T *vals, int numvals)
    {
        int read = min(maxlen-len, numvals);
        if (read<numvals) flags |= OVERREAD;
        memcpy(vals, &buf[len], read*sizeof(T));
        len += read;
        return read;
    }

    int length() const { return len; }
    int maxlength() const { return maxlen; }
    int remaining() const { return maxlen-len; }
    bool overread() const { return flags&OVERREAD; }
    bool overwrote() const { return flags&OVERWROTE; }

    void forceoverread()
    {
        len = maxlen;
        flags |= OVERREAD;
    }
};

typedef databuf<char> charbuf;
typedef databuf<uchar> ucharbuf;

template <class T> struct vector
{
    static const int MINSIZE = 8;

    T *buf;
    int alen, ulen;

    vector() : buf(NULL), alen(0), ulen(0)
    {
    }

    vector(const vector &v) : buf(NULL), alen(0), ulen(0)
    {
        *this = v;
    }

    ~vector() { setsize(0); if (buf) delete[](uchar *)buf; }

    vector<T> &operator=(const vector<T> &v)
    {
        setsize(0);
        if (v.length() > alen) vrealloc(v.length());
        loopv(v) add(v[i]);
        return *this;
    }

    T &add(const T &x)
    {
        if (ulen==alen) vrealloc(ulen+1);
        new(&buf[ulen]) T(x);
        return buf[ulen++];
    }

    T &add()
    {
        if (ulen==alen) vrealloc(ulen+1);
        new(&buf[ulen]) T;
        return buf[ulen++];
    }

    T &dup()
    {
        if (ulen==alen) vrealloc(ulen+1);
        new(&buf[ulen]) T(buf[ulen-1]);
        return buf[ulen++];
    }

    void move(vector<T> &v)
    {
        if (!ulen)
        {
            swap(buf, v.buf);
            swap(ulen, v.ulen);
            swap(alen, v.alen);
        }
        else
        {
            vrealloc(ulen+v.ulen);
            if (v.ulen) memcpy(&buf[ulen], v.buf, v.ulen*sizeof(T));
            ulen += v.ulen;
            v.ulen = 0;
        }
    }

    bool inrange(size_t i) const { return i<size_t(ulen); }
    bool inrange(int i) const { return i>=0 && i<ulen; }

    T &pop() { return buf[--ulen]; }
    T &last() { return buf[ulen-1]; }
    void drop() { buf[--ulen].~T(); }
    bool empty() const { return ulen==0; }

    int length() const { return ulen; }
    T &operator[](int i) { ASSERT(i>=0 && i<ulen); return buf[i]; }
    const T &operator[](int i) const { ASSERT(i >= 0 && i<ulen); return buf[i]; }

    void setsize(int i)         { ASSERT(i<=ulen); while (ulen>i) drop(); }
    void setsizenodelete(int i) { ASSERT(i<=ulen); ulen = i; }

    void deletecontentsp() { while (!empty()) delete   pop(); }
    void deletecontentsa() { while (!empty()) delete[] pop(); }

    T *getbuf() { return buf; }
    const T *getbuf() const { return buf; }
    bool inbuf(const T *e) const { return e >= buf && e < &buf[ulen]; }

    template<class ST>
    void sort(int (__cdecl *cf)(ST *, ST *), int i = 0, int n = -1)
    {
        qsort(&buf[i], n<0 ? ulen : n, sizeof(T), (int (__cdecl *)(const void *, const void *))cf);
    }

    void vrealloc(int sz)
    {
        int olen = alen;
        if (!alen) alen = max(MINSIZE, sz);
        else while (alen < sz) alen *= 2;
        if (alen <= olen) return;
        uchar *newbuf = new uchar[alen*sizeof(T)];
        if (olen > 0)
        {
            memcpy(newbuf, buf, olen*sizeof(T));
            delete[](uchar *)buf;
        }
        buf = (T *)newbuf;
    }

    databuf<T> reserve(int sz)
    {
        if (ulen+sz > alen) vrealloc(ulen+sz);
        return databuf<T>(&buf[ulen], sz);
    }

    void advance(int sz)
    {
        ulen += sz;
    }

    void addbuf(const databuf<T> &p)
    {
        advance(p.length());
    }

    void put(const T *v, int n)
    {
        databuf<T> buf = reserve(n);
        buf.put(v, n);
        addbuf(buf);
    }

    void remove(int i, int n)
    {
        for (int p = i+n; p<ulen; p++) buf[p-n] = buf[p];
        ulen -= n;
    }

    T remove(int i)
    {
        T e = buf[i];
        for (int p = i+1; p<ulen; p++) buf[p-1] = buf[p];
        ulen--;
        return e;
    }

    template<class U>
    int find(const U &o)
    {
        loopi(ulen) if (buf[i]==o) return i;
        return -1;
    }

    void removeobj(const T &o)
    {
        loopi(ulen) if (buf[i]==o) remove(i--);
    }

    void replacewithlast(const T &o)
    {
        if (!ulen) return;
        loopi(ulen-1) if (buf[i]==o)
        {
            buf[i] = buf[ulen-1];
        }
        ulen--;
    }

    T &insert(int i, const T &e)
    {
        add(T());
        for (int p = ulen-1; p>i; p--) buf[p] = buf[p-1];
        buf[i] = e;
        return buf[i];
    }

    T *insert(int i, const T *e, int n)
    {
        if (ulen+n>alen) vrealloc(ulen+n);
        loopj(n) add(T());
        for (int p = ulen-1; p>=i+n; p--) buf[p] = buf[p-n];
        loopj(n) buf[i+j] = e[j];
        return &buf[i];
    }

    void reverse()
    {
        loopi(ulen/2) swap(buf[i], buf[ulen-1-i]);
    }
};

void putint(ucharbuf &p, int n);
int getint(ucharbuf &p);
int getuint(ucharbuf &p);
void putstring(const char *t, ucharbuf &p);
int getstring(char *text, ucharbuf &p, int len=_MAXDEFSTR);

int iscubeprint(int c);
int uni2cube(int c);
int cube2uni(int c);
int decodeutf8(uchar *dst, uchar *src, int len, int *carry = NULL);
int encodeutf8(uchar *dstbuf, int dstlen, uchar *srcbuf, int srclen, int *carry = NULL);

#endif // CUBE_TOOLS_H

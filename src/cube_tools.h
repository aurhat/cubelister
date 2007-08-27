// generic useful stuff for any C++ program

#ifndef CUBE_TOOLS_H
#define CUBE_TOOLS_H

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

#define swapv(t,a,b) { t m=a; a=b; b=m; }
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#define loop(v,m) for(int v = 0; v<int(m); v++)
#define loopi(m) loop(i,m)
#define loopj(m) loop(j,m)
#define loopk(m) loop(k,m)
#define loopl(m) loop(l,m)

#ifdef WIN32
#define PATHDIV wxT("\\")
#define PATHDIVA wxT('\\')
#else
#define __cdecl
#define _vsnprintf vsnprintf
#define PATHDIV wxT("/")
#define PATHDIVA wxT('/')
#endif

// easy safe strings

#define _MAXDEFSTR 260
typedef char string[_MAXDEFSTR];

///inline void formatstring(char *d, const char *fmt, va_list v) { _vsnprintf(d, _MAXDEFSTR, fmt, v); d[_MAXDEFSTR-1] = 0; }
inline char *s_strncpy(char *d, const char *s, size_t m)
{
    strncpy(d,s,m);
    d[m-1] = 0;
    return d;
}
inline char *s_strcpy(char *d, const char *s)
{
    return s_strncpy(d,s,_MAXDEFSTR);
}
inline char *s_strcat(char *d, const char *s)
{
    size_t n = strlen(d);
    return s_strncpy(d+n,s,_MAXDEFSTR-n);
}


#define s_sprintf(d) s_sprintf_f((char *)d)
#define s_sprintfd(d) string d; s_sprintf(d)
#define s_sprintfdlv(d,last,fmt) string d; { va_list ap; va_start(ap, last); formatstring(d, fmt, ap); va_end(ap); }
#define s_sprintfdv(d,fmt) s_sprintfdlv(d,fmt,fmt)


template <class T> void _swapv(T &a, T &b)
{
    T t = a;
    a = b;
    b = t;
}


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

    int length() const
    {
        return len;
    }
    int maxlength() const
    {
        return maxlen;
    }
    int remaining() const
    {
        return maxlen-len;
    }
    bool overread() const
    {
        return flags&OVERREAD;
    }
    bool overwrote() const
    {
        return flags&OVERWROTE;
    }

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
    T *buf;
    int alen;
    int ulen;

    vector()
    {
        alen = 8;
        buf = (T *)new uchar[alen*sizeof(T)];
        ulen = 0;
    }
    vector(const vector<T> &v)
    {
        alen = v.length();
        buf = (T *)new uchar[alen*sizeof(T)];
        ulen = 0;
        *this = v;
    }

    ~vector()
    {
        setsize(0);
        delete[](uchar *)buf;
    }

    vector<T> &operator=(const vector<T> &v)
    {
        setsize(0);
        loopv(v) add(v[i]);
        return *this;
    }

    T &at(const int i) const
    {
        ASSERT(i >= 0 && i<ulen);
        return buf[i];
    }

    T &add(const T &x)
    {
        if (ulen==alen) vrealloc();
        new(&buf[ulen]) T(x);
        return buf[ulen++];
    }

    T &add()
    {
        if (ulen==alen) vrealloc();
        new(&buf[ulen]) T;
        return buf[ulen++];
    }

    T &dup()
    {
        if (ulen==alen) vrealloc();
        new(&buf[ulen]) T(buf[ulen-1]);
        return buf[ulen++];
    }

    void move(vector<T> &v)
    {
        if (!ulen)
        {
            swapv(T *, buf, v.buf);
            swapv(int, ulen, v.ulen);
            swapv(int, alen, v.alen);
        }
        else
        {
            while (alen-ulen<v.ulen) vrealloc();
            memcpy(&buf[ulen], v.buf, v.ulen*sizeof(T));
            ulen += v.ulen;
            v.ulen = 0;
        }
    }

    bool inrange(size_t i) const
    {
        return i<size_t(ulen);
    }
    bool inrange(int i) const
    {
        return i>=0 && i<ulen;
    }

    T &pop()
    {
        return buf[--ulen];
    }
    T &last()
    {
        return buf[ulen-1];
    }
    void drop()
    {
        buf[--ulen].~T();
    }
    bool empty() const
    {
        return ulen==0;
    }

    int length() const
    {
        return ulen;
    }
    T &operator[](int i)
    {
        ASSERT(i>=0 && i<ulen);
        return buf[i];
    }
    const T &operator[](int i) const
    {
        ASSERT(i >= 0 && i<ulen);
        return buf[i];
    }

    void setsize(int i)
    {
        ASSERT(i<=ulen);
        while (ulen>i) drop();
    }
    void setsizenodelete(int i)
    {
        ASSERT(i<=ulen);
        ulen = i;
    }

    void deletecontentsp()
    {
        while (!empty()) delete   pop();
    }
    void deletecontentsa()
    {
        while (!empty()) delete[] pop();
    }

    T *getbuf()
    {
        return buf;
    }
    const T *getbuf() const
    {
        return buf;
    }

    template<class ST>
    void sort(int (__cdecl *cf)(ST *, ST *), int i = 0, int n = -1)
    {
        qsort(&buf[i], n<0?ulen:n, sizeof(T), (int (__cdecl *)(const void *,const void *))cf);
    }

    void *_realloc(void *p, int oldsize, int newsize)
    {
        void *np = new uchar[newsize];
        memcpy(np, p, newsize>oldsize ? oldsize : newsize);
        delete[](uchar *)p;
        return np;
    }

    void vrealloc()
    {
        int olen = alen;
        buf = (T *)_realloc(buf, olen*sizeof(T), (alen *= 2)*sizeof(T));
    }

    databuf<T> reserve(int sz)
    {
        while (alen-ulen<sz) vrealloc();
        return databuf<T>(&buf[ulen], sz);
    }

    void addbuf(const databuf<T> &p)
    {
        ulen += p.length();
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

    int find(const T &o)
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

    void reverse()
    {
        loopi(ulen/2) swapv(T, buf[i], buf[ulen-1-i]);
    }
};

#endif // CUBE_TOOLS_H


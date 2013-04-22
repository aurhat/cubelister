#ifndef CUBE_TOOLS_H
#define CUBE_TOOLS_H

#define MAXSTRLEN 260

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;

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

CSL_DLL_TOOLS void putint(ucharbuf &p, int n);
CSL_DLL_TOOLS int getint(ucharbuf &p);
CSL_DLL_TOOLS int getuint(ucharbuf &p);
CSL_DLL_TOOLS void putstring(const char *t, ucharbuf &p);
CSL_DLL_TOOLS int getstring(char *text, ucharbuf &p, int len = MAXSTRLEN);

CSL_DLL_TOOLS int iscubeprint(int c);
CSL_DLL_TOOLS int cube2uni(int c);
CSL_DLL_TOOLS int encodeutf8(uchar *dstbuf, int dstlen, const uchar *srcbuf, int srclen, int *carry);

#endif // CUBE_TOOLS_H

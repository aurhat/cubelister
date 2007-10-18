struct rpgdummycom : iclientcom
{
    ~rpgdummycom() {}

    void gamedisconnect() {}
    void parsepacketclient(int chan, ucharbuf &p) {}
    int sendpacketclient(ucharbuf &p, bool &reliable, dynent *d) { return -1; }
    void gameconnect(bool _remote) {}
    bool allowedittoggle() { return true; }
    void writeclientinfo(FILE *f) {}
    void toserver(char *text) {}
    void changemap(const char *name) { load_world(name); }
};

struct rpgdummyserver : igameserver
{
    ~rpgdummyserver() {}

    void *newinfo() { return NULL; }
    void deleteinfo(void *ci) {}
    void serverinit() {}
    void clientdisconnect(int n) {}
    int clientconnect(int n, uint ip) { return DISC_NONE; }
    void localdisconnect(int n) {}
    void localconnect(int n) {}
    char *servername() { return "foo"; }
    void parsepacket(int sender, int chan, bool reliable, ucharbuf &p) {}
    bool sendpackets() { return false; }
    int welcomepacket(ucharbuf &p, int n) { return -1; }
    void serverinforeply(ucharbuf &p) {}
    void serverupdate(int lastmillis, int totalmillis) {}
    bool servercompatible(char *name, char *sdec, char *map, int ping, const vector<int> &attr, int np) { return false; }
    void serverinfostr(char *buf, const char *name, const char *desc, const char *map, int ping, const vector<int> &attr, int np) {}
    int serverinfoport() { return 0; }
    int serverport() { return 0; }
    char *getdefaultmaster() { return "localhost"; }
    void sendservmsg(const char *s) {}
    //***************** BEGIN OF MODIFICATION **********************
    bool serverinfostats(ucharbuf &p, int num) { return false; } //NEW
    void serverinfoteamscore(ucharbuf &p) {} //NEW
    //***************** END OF MODIFICATION ************************
};

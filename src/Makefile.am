
AM_CXXFLAGS = -DLOCALEDIR=\"$(localedir)\" -DGEOIPDATABASEDIR=\"$(datadir)/csl\"

bin_PROGRAMS = csl
csl_SOURCES = CslDlgAddMaster.cpp CslDlgAddServer.cpp CslDlgConnectWait.cpp \
	CslDlgSettings.cpp CslEngine.cpp CslFrame.cpp CslGame.cpp CslListCtrlInfo.cpp \
	CslListCtrlServer.cpp CslStatusBar.cpp CslUDP.cpp

# set the include path found by configure
AM_CPPFLAGS =  $(LIBSDL_CFLAGS) $(all_includes)

# the library search path.
csl_LDFLAGS = $(all_libraries) $(LIBSDL_RPATH)
csl_LDADD = $(LIBSDL_LIBS) $(top_builddir)/src/libGeoIP/libGeoIP.a ${WX_LIBS}
noinst_HEADERS = CslDlgAddMaster.h CslDlgAddServer.h CslDlgConnectWait.h \
	CslDlgSettings.h CslFrame.h CslGame.h CslListCtrlInfo.h CslListCtrlServer.h CslMenu.h \
	CslSettings.h CslStatusBar.h CslUDP.h tools.h
SUBDIRS = libGeoIP img/flags
INCLUDES = -I$(top_srcdir)/src/libGeoIP -I$(top_srcdir)/src/img/flags

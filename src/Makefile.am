
AM_CXXFLAGS = -DLOCALEDIR=\"$(localedir)\" -DDATADIR=\"$(datadir)/csl\"

bin_PROGRAMS = csl
csl_SOURCES = CslArt.cpp CslDlgAddMaster.cpp CslDlgAddServer.cpp \
	CslDlgConnectPass.cpp CslDlgConnectWait.cpp CslDlgExtended.cpp CslDlgOutput.cpp \
	CslDlgSettings.cpp CslEngine.cpp CslExtendedInfo.cpp CslFrame.cpp CslGame.cpp \
	CslListCtrlInfo.cpp CslListCtrlServer.cpp CslMenu.cpp CslStatusBar.cpp CslTools.cpp \
	CslUDP.cpp cube_tools.cpp

# set the include path found by configure
AM_CPPFLAGS =  $(LIBSDL_CFLAGS) $(all_includes)

# the library search path.
csl_LDFLAGS = $(all_libraries)
csl_LDADD = $(top_builddir)/src/libGeoIP/libGeoIP.a ${WX_LIBS}
noinst_HEADERS = CslArt.h CslDlgAddMaster.h CslDlgAddServer.h \
	CslDlgConnectPass.h CslDlgConnectWait.h CslDlgExtended.h CslDlgOutput.h CslDlgSettings.h \
	CslExtendedInfo.h CslFrame.h CslGame.h CslListCtrlInfo.h CslListCtrlServer.h CslMenu.h \
	CslSettings.h CslStatusBar.h CslTools.h CslUDP.h cube_tools.h
SUBDIRS = cslmapcfgtool libGeoIP
INCLUDES = -I$(top_srcdir)/src/libGeoIP -I$(top_srcdir)/$(CSL_FLAGS_INCLUDE)

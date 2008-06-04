
AM_CXXFLAGS = -DLOCALEDIR=\"$(localedir)\" -DDATADIR=\"$(datadir)/csl\"

bin_PROGRAMS = csl
if build_cslmapcfgtool
bin_PROGRAMS += cslmapcfgtool
endif
csl_SOURCES = CslArt.cpp CslDlgAbout.cpp CslDlgAddMaster.cpp \
	CslDlgAddServer.cpp CslDlgConnectPass.cpp CslDlgConnectWait.cpp CslDlgExtended.cpp \
	CslDlgOutput.cpp CslDlgSettings.cpp CslDlgTraffic.cpp CslEngine.cpp CslExtendedInfo.cpp \
	CslFrame.cpp CslGame.cpp CslGeoIP.cpp CslListCtrlInfo.cpp CslListCtrlPlayer.cpp \
	CslListCtrlServer.cpp CslMenu.cpp CslPanelMap.cpp CslStatusBar.cpp CslTools.cpp CslUDP.cpp \
	cube_tools.cpp

# set the include path found by configure
AM_CPPFLAGS =  $(LIBSDL_CFLAGS) $(all_includes)

# the library search path.
csl_LDFLAGS = $(all_libraries)
csl_LDADD = ${WX_LIBS} ${GEOIP_LIBS}
noinst_HEADERS = CslArt.h CslDlgAbout.h CslDlgAddMaster.h CslDlgAddServer.h \
	CslDlgConnectPass.h CslDlgConnectWait.h CslDlgExtended.h CslDlgOutput.h CslDlgSettings.h \
	CslDlgTraffic.h CslExtendedInfo.h CslFrame.h CslGame.h CslGeoIP.h CslLicense.h \
	CslListCtrlInfo.h CslListCtrlPlayer.h CslListCtrlServer.h CslMapCfgTool.h CslMenu.h \
	CslPanelMap.h CslSettings.h CslStatusBar.h CslTools.h CslUDP.h csl_icon_png.h \
	cube_tools.h
SUBDIRS = libGeoIP
INCLUDES = -I$(top_srcdir)/src/libGeoIP -I$(top_srcdir)/src/img/flags
cslmapcfgtool_SOURCES = CslMapCfgTool.cpp CslPanelMap.cpp CslTools.cpp
cslmapcfgtool_LDADD = ${WX_LIBS}

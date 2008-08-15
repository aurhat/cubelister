
bin_PROGRAMS = csl
if build_cslmapcfgtool
bin_PROGRAMS += cslmapcfgtool
endif

AM_CXXFLAGS = -DLOCALEDIR=\"$(localedir)\" -DDATADIR=\"$(datadir)/csl\"

# set the include path found by configure
AM_CPPFLAGS =  $(all_includes)

# the library search path.
csl_LDFLAGS = $(all_libraries)
csl_LDADD = $(top_builddir)/src/engine/libcslengine.a \
	$(top_builddir)/src/engine/libcsltools.a libcslpanelmap.a ${GEOIP_LIBS} ${WX_LIBS}
SUBDIRS = libGeoIP engine
INCLUDES = -I$(top_srcdir)/src/libGeoIP -I$(top_srcdir)/src/img/flags
cslmapcfgtool_SOURCES = CslMapCfgTool.cpp
cslmapcfgtool_LDADD = libcslpanelmap.a \
	$(top_builddir)/src/engine/libcsltools.a ${WX_LIBS}
csl_SOURCES = CslArt.cpp CslConnectionState.cpp CslDlgAbout.cpp \
	CslDlgAddMaster.cpp CslDlgAddServer.cpp CslDlgConnectPass.cpp CslDlgConnectWait.cpp \
	CslDlgExtended.cpp CslDlgOutput.cpp CslDlgSettings.cpp CslDlgTraffic.cpp CslFrame.cpp \
	CslGeoIP.cpp CslListCtrlInfo.cpp CslListCtrlPlayer.cpp CslListCtrlServer.cpp \
	CslMenu.cpp CslStatusBar.cpp
noinst_LIBRARIES = libcslpanelmap.a
libcslpanelmap_a_SOURCES = CslPanelMap.cpp

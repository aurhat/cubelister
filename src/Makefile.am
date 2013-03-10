
SUBDIRS = pch plugin engine tools external plugins

bin_PROGRAMS = csl
if build_cslmapcfgtool
bin_PROGRAMS += cslmapcfgtool
endif

AM_CPPFLAGS = -DPROJECTDIR=\"$(abs_top_srcdir)\" -DBUILDDIR=\"$(abs_builddir)\" \
	-DLOCALEDIR=\"$(localedir)\" -DDATADIR=\"$(datadir)/csl\" -DPKGLIBDIR=\"$(pkglibdir)\"

# the library search path.
csl_LDADD = $(top_builddir)/src/engine/libcslengine.la \
	$(top_builddir)/src/engine/tools/libcsltools.la \
	$(top_builddir)/src/plugin/libcslplugin.la $(top_builddir)/src/tools/libcslguitools.la \
	${WX_LIBS}
INCLUDES = -I$(top_srcdir)/src/engine -I$(top_srcdir)/src/engine/tools \
	-I$(top_srcdir)/src/plugin -I$(top_srcdir)/src/tools \
	-I$(top_builddir)/src/pch -I$(top_srcdir)/src/img/flags
cslmapcfgtool_SOURCES = CslMapCfgTool.cpp
cslmapcfgtool_LDADD = $(top_builddir)/src/tools/libcslguitools.la \
	$(top_builddir)/src/engine/tools/libcsltools.la ${WX_LIBS}
csl_SOURCES = CslApp.cpp CslDlgAbout.cpp CslDlgAddMaster.cpp \
	CslDlgAddServer.cpp CslDlgConnectPass.cpp CslDlgConnectWait.cpp CslDlgExtended.cpp \
	CslDlgOutput.cpp CslDlgSettings.cpp CslDlgTraffic.cpp CslFrame.cpp CslGameConnection.cpp \
	CslGameProcess.cpp CslIPC.cpp CslListCtrlCountry.cpp CslListCtrlInfo.cpp \
	CslPanelServerView.cpp CslListCtrlPlayerSearch.cpp CslListCtrlServer.cpp CslSettings.cpp \
	CslStatusBar.cpp CslVersionCheck.cpp
noinst_HEADERS = CslApp.h CslGameConnection.h \
	CslListCtrlPlayerSearch.h

SUBDIRS = pch plugin engine tools plugins

bin_PROGRAMS = csl cslmapcfgtool

AM_CPPFLAGS = -DPROJECTDIR=\"$(abs_top_srcdir)\" -DBUILDDIR=\"$(abs_builddir)\" \
	-DLOCALEDIR=\"$(localedir)\" -DDATADIR=\"$(datadir)/csl\" -DPKGLIBDIR=\"$(pkglibdir)\" \
	-I$(top_srcdir)/src/engine -I$(top_srcdir)/src/engine/tools \
	-I$(top_srcdir)/src/plugin -I$(top_srcdir)/src/tools \
	-I$(top_builddir)/src/pch -I$(top_srcdir)/src/img/flags

cslmapcfgtool_SOURCES = CslMapCfgTool.cpp
cslmapcfgtool_LDADD = $(top_builddir)/src/tools/libcslguitools.la \
	$(top_builddir)/src/engine/tools/libcsltools.la ${WX_LIBS}

csl_SOURCES = CslApp.cpp CslDlgAbout.cpp CslDlgAddMaster.cpp \
	CslDlgAddServer.cpp CslDlgConnectPass.cpp CslDlgConnectWait.cpp CslDlgExtended.cpp \
	CslDlgOutput.cpp CslDlgSettings.cpp CslFrame.cpp CslGameConnection.cpp \
	CslGameProcess.cpp CslIPC.cpp CslPanelCountryView.cpp CslPanelGameSettings.cpp \
	CslPanelPlayerSearch.cpp CslPanelSearch.cpp CslPanelServerView.cpp CslPanelTraffic.cpp \
	CslServerBrowser.cpp CslServerInfo.cpp CslSettings.cpp CslStatusBar.cpp

csl_LDADD = $(top_builddir)/src/engine/libcslengine.la \
	$(top_builddir)/src/engine/tools/libcsltools.la \
	$(top_builddir)/src/plugin/libcslplugin.la $(top_builddir)/src/tools/libcslguitools.la \
	${WX_LIBS}

noinst_HEADERS = CslApp.h CslGameConnection.h CslPanelSearch.h


SUBDIRS = pch plugin engine tools external irc plugins

bin_PROGRAMS = csl
if build_cslmapcfgtool
bin_PROGRAMS += cslmapcfgtool
endif

AM_CPPFLAGS = -DPROJECTDIR=\"$(abs_top_srcdir)\" -DBUILDDIR=\"$(abs_builddir)\" \
	-DLOCALEDIR=\"$(localedir)\" -DDATADIR=\"$(datadir)/csl\" -DPKGLIBDIR=\"$(pkglibdir)\"

# the library search path.
csl_LDADD = $(top_builddir)/src/engine/libcslengine.la \
	$(top_builddir)/src/engine/tools/libcsltools.la $(top_builddir)/src/irc/libcslircengine.la \
	$(top_builddir)/src/plugin/libcslplugin.la $(top_builddir)/src/tools/libcslguitools.la \
	${SPEECHD_LIBS} ${WX_LIBS}
INCLUDES = -I$(top_srcdir)/src/engine -I$(top_srcdir)/src/engine/tools \
	-I$(top_srcdir)/src/plugin -I$(top_srcdir)/src/tools \
	-I$(top_builddir)/src/pch -I$(top_srcdir)/src/img/flags
cslmapcfgtool_SOURCES = CslMapCfgTool.cpp
cslmapcfgtool_LDADD = $(top_builddir)/src/tools/libcslguitools.la \
	$(top_builddir)/src/engine/tools/libcsltools.la ${WX_LIBS}
csl_SOURCES = CslApp.cpp CslDlgAbout.cpp CslDlgAddMaster.cpp \
	CslDlgAddServer.cpp CslDlgConnectPass.cpp CslDlgConnectWait.cpp CslDlgExtended.cpp \
	CslDlgOutput.cpp CslDlgSettings.cpp CslDlgTraffic.cpp CslFrame.cpp CslGameConnection.cpp \
	CslGameProcess.cpp CslIPC.cpp CslIRC.cpp CslListCtrlCountry.cpp CslListCtrlInfo.cpp \
	CslListCtrlPlayer.cpp CslListCtrlPlayerSearch.cpp CslListCtrlServer.cpp CslSettings.cpp \
	CslStatusBar.cpp CslTTS.cpp CslVersionCheck.cpp CslWizardGameSettings.cpp
noinst_HEADERS = CslApp.h CslGameConnection.h CslIRC.h \
	CslListCtrlPlayerSearch.h CslTTS.h CslWizardGameSettings.h

if with_internal_libircclient
INCLUDES += -I$(top_srcdir)/src/external/libircclient/include#include
endif

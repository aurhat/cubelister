METASOURCES = AUTO

SUBDIRS = tools

INCLUDES = -I$(top_srcdir)/src/engine/tools -I$(top_srcdir)/src/plugin \
	-I$(top_srcdir)/src/tools -I$(top_builddir)/src/pch

lib_LTLIBRARIES = libcslengine.la

libcslengine_la_SOURCES = CslDNSResolver.cpp CslEngine.cpp CslExtendedInfo.cpp \
	CslGame.cpp CslUDP.cpp

include_HEADERS = CslDNSResolver.h CslEngine.h CslExtendedInfo.h CslGame.h \
	CslUDP.h CslVersion.h

libcslengine_la_LIBADD = $(top_builddir)/src/engine/tools/libcsltools.la \
	$(top_builddir)/src/plugin/libcslplugin.la ${WX_LIBS}

libcslengine_la_LDFLAGS = -version-info ${PKG_LIB_VERSION}

METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/src/engine/tools -I$(top_srcdir)/src/plugin \
	-I$(top_srcdir)/src/tools -I$(top_builddir)/src/pch

lib_LTLIBRARIES = libcslengine.la
libcslengine_la_SOURCES = CslEngine.cpp CslExtendedInfo.cpp CslGame.cpp \
	CslUDP.cpp
libcslengine_la_LIBADD = $(top_builddir)/src/engine/tools/libcsltools.la \
	$(top_builddir)/src/plugin/libcslplugin.la ${WX_LIBS}
SUBDIRS = tools
include_HEADERS = CslEngine.h CslExtendedInfo.h CslGame.h CslUDP.h CslVersion.h

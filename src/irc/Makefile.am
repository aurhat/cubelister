
METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/engine \
	-I$(top_srcdir)/src/engine/tools -I$(top_srcdir)/src/tools \
	-I$(top_builddir)/src/pch

lib_LTLIBRARIES = libcslircengine.la
libcslircengine_la_SOURCES = CslIRCEngine.cpp
noinst_HEADERS = CslIRCEngine.h

if with_internal_libircclient
INCLUDES += -I$(top_srcdir)/src/external/libircclient/include
libcslircengine_la_LIBADD = $(top_builddir)/src/external/libircclient/src/libircclient.la
else
libcslircengine_la_LIBADD = -lircclient
endif

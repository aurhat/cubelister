INCLUDES = -I$(top_srcdir)/src/engine -I$(top_srcdir)/src/engine/tools \
	-I$(top_srcdir)/src/plugin -I$(top_srcdir)/src/tools -I$(top_builddir)/src/pch
METASOURCES = AUTO
plugindir = $(pkglibdir)/plugins
plugin_LTLIBRARIES = sauerbraten.la
sauerbraten_la_LDFLAGS = -avoid-version -module
sauerbraten_la_SOURCES = CslGameSauerbraten.cpp
sauerbraten_la_LIBADD = $(top_builddir)/src/engine/tools/libcsltools.la

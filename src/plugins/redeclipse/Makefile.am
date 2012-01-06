INCLUDES = -I$(top_srcdir)/src/engine -I$(top_srcdir)/src/engine/tools \
	-I$(top_srcdir)/src/plugin -I$(top_srcdir)/src/tools -I$(top_builddir)/src/pch
METASOURCES = AUTO
plugindir = $(pkglibdir)/plugins
plugin_LTLIBRARIES = redeclipse.la
redeclipse_la_LDFLAGS = -avoid-version -module
redeclipse_la_SOURCES = CslGameRedEclipse.cpp
redeclipse_la_LIBADD = $(top_builddir)/src/engine/tools/libcsltools.la

AM_CPPFLAGS = -I$(top_srcdir)/src/engine -I$(top_srcdir)/src/engine/tools \
	-I$(top_srcdir)/src/plugin -I$(top_srcdir)/src/tools -I$(top_builddir)/src/pch
METASOURCES = AUTO
plugindir = $(pkglibdir)/plugins
plugin_LTLIBRARIES = assaultcube.la
assaultcube_la_LDFLAGS = -avoid-version -module
assaultcube_la_SOURCES = CslGameAssaultCube.cpp
assaultcube_la_LIBADD = $(top_builddir)/src/engine/tools/libcsltools.la

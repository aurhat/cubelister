AM_CPPFLAGS = -I$(top_srcdir)/src/engine -I$(top_srcdir)/src/engine/tools \
	-I$(top_srcdir)/src/plugin -I$(top_srcdir)/src/tools -I$(top_builddir)/src/pch
METASOURCES = AUTO
plugindir = $(pkglibdir)/plugins
plugin_LTLIBRARIES = tesseract.la
tesseract_la_LDFLAGS = -avoid-version -module
tesseract_la_SOURCES = CslGameTesseract.cpp
tesseract_la_LIBADD = $(top_builddir)/src/engine/tools/libcsltools.la

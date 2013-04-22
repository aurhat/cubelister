INCLUDES = -I$(top_srcdir)/src/engine -I$(top_srcdir)/src/tools \
	-I$(top_builddir)/src/pch

METASOURCES = AUTO

lib_LTLIBRARIES = libcsltools.la
libcsltools_la_SOURCES = CslCharEncoding.cpp CslCubeEngineTools.cpp \
	CslIPV4Addr.cpp CslProtocolInput.cpp CslTools.cpp
include_HEADERS = CslCharEncoding.h CslCubeEngineTools.h CslDynlib.h \
	CslIPV4Addr.h CslProtocolInput.h CslTools.h

libcsltools_la_LDFLAGS = -version-info ${PKG_LIB_VERSION}
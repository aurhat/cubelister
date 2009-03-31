
AM_CXXFLAGS = -DLOCALEDIR=\"$(localedir)\" -DDATADIR=\"$(datadir)/csl\"

METASOURCES = AUTO

noinst_HEADERS = CslCharEncoding.h CslEngine.h CslExtendedInfo.h CslGame.h \
	CslGameAssaultCube.h CslGameBloodFrontier.h CslGameCube.h CslGameSauerbraten.h CslTools.h \
	CslUDP.h cube_tools.h
noinst_LIBRARIES = libcslengine.a libcsltools.a
libcsltools_a_SOURCES = CslCharEncoding.cpp CslTools.cpp
libcslengine_a_SOURCES = CslEngine.cpp CslExtendedInfo.cpp CslGame.cpp \
	CslGameAssaultCube.cpp CslGameBloodFrontier.cpp CslGameCube.cpp CslGameSauerbraten.cpp \
	CslUDP.cpp cube_tools.cpp

INCLUDES = -I$(top_builddir)/src/pch

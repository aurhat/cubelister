AM_CXXFLAGS = -DLOCALEDIR=\"$(localedir)\" -DDATADIR=\"$(datadir)/csl\"

METASOURCES = AUTO
noinst_HEADERS = CslEngine.h CslExtendedInfo.h CslGame.h CslGameAssaultCube.h \
	CslGameBloodFrontier.h CslGameCube.h CslGameSauerbraten.h CslTools.h CslUDP.h CslVersion.h \
	cube_tools.h
noinst_LIBRARIES = libcslengine.a libcsltools.a
libcsltools_a_SOURCES = CslTools.cpp
libcslengine_a_SOURCES = CslEngine.cpp CslExtendedInfo.cpp CslGame.cpp \
	CslGameAssaultCube.cpp CslGameBloodFrontier.cpp CslGameCube.cpp CslGameSauerbraten.cpp \
	CslUDP.cpp cube_tools.cpp

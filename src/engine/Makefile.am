INCLUDES = -I$(top_srcdir)/src/img/flags -I$(top_srcdir)/src/libGeoIP
METASOURCES = AUTO
lib_LIBRARIES = libcsltools.a libcslengine.a
libcslengine_a_SOURCES = CslEngine.cpp CslExtendedInfo.cpp CslGame.cpp \
	CslGameAssaultCube.cpp CslGameBloodFrontier.cpp CslGameCube.cpp CslGameSauerbraten.cpp \
	CslUDP.cpp cube_tools.cpp
noinst_HEADERS = CslEngine.h CslExtendedInfo.h CslGame.h CslGameAssaultCube.h \
	CslGameBloodFrontier.h CslGameCube.h CslGameSauerbraten.h CslTools.h CslUDP.h CslVersion.h \
	cube_tools.h
libcsltools_a_SOURCES = CslTools.cpp

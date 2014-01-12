METASOURCES = AUTO

AM_CPPFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/src/engine \
	-I$(top_srcdir)/src/engine/tools -I$(top_srcdir)/src/irc/libircclient/include \
	-I$(top_srcdir)/src/plugin -I$(top_builddir)/src/pch \
	-I$(top_srcdir)/src/img/flags

lib_LTLIBRARIES = libcslguitools.la
libcslguitools_la_SOURCES = CslArt.cpp CslGeoIP.cpp CslGuiTools.cpp CslListCtrl.cpp \
	CslMenu.cpp CslPanelMap.cpp CslTTS.cpp CslToolTip.cpp
libcslguitools_la_LIBADD = $(top_builddir)/src/engine/libcslengine.la \
	$(top_builddir)/src/plugin/libcslplugin.la $(top_builddir)/src/engine/tools/libcsltools.la \
	${WX_LIBS}
include_HEADERS = CslArt.h CslGeoIP.h CslGuiTools.h \
	CslListCtrl.h CslMenu.h CslPanelMap.h \
	CslTTS.h CslToolTip.h

libcslguitools_la_LIBADD += ${SPEECHD_LIBS}
libcslguitools_la_LDFLAGS = -version-info ${PKG_LIB_VERSION}

if WITH_INTERNAL_GEOIP
AM_CPPFLAGS += -I$(top_srcdir)/src/external/libGeoIP
libcslguitools_la_SOURCES += $(top_srcdir)/src/external/libGeoIP/GeoIPInternal.c
else
libcslguitools_la_LIBADD += -lGeoIP
endif

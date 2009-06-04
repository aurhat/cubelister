csldatadir = $(datadir)/csl
if with_internal_geoipdatabase
csldata_DATA += GeoIP.dat
endif

desktopdir = $(datadir)/applications
desktop_in_files = csl.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

pixmapsdir = $(datadir)/pixmaps
pixmaps_DATA = csl.png

EXTRA_DIST = $(csldata_DATA) $(desktop_DATA) $(pixmaps_DATA)

SUBDIRS = maps

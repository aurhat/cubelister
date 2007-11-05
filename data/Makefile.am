datadir =	$(datadir)/csl
data_DATA = GeoIP.dat csl_start_sb.ogz
EXTRA_DIST = $(data_DATA)

appsdir = $(datadir)/applications
apps_in_files = csl.desktop.in
apps_DATA = $(apps_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

pixmapsdir = $(datadir)/pixmaps
pximaps_DATA = csl.png

EXTRA_DIST = $(data_DATA) $(pixmaps_DATA)

SUBDIRS = maps

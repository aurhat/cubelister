SUBDIRS = maps

CLEANFILES = *.desktop

desktopdir = $(datadir)/applications
desktop_in_files = csl.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

pixmapsdir = $(datadir)/pixmaps
pixmaps_DATA = csl.png

EXTRA_DIST = $(desktop_DATA) $(pixmaps_DATA)

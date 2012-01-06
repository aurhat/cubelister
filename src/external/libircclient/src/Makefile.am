
if with_internal_libircclient
noinst_LTLIBRARIES = libircclient.la
libircclient_la_SOURCES = libircclient.c
INCLUDES = -I$(top_srcdir)/src/external/libircclient/include
AM_CFLAGS = -DIN_BUILDING_LIBIRC -DENABLE_THREADS -D_REENTRANT
endif

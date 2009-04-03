
if with_internal_libircclient
noinst_LIBRARIES = libircclient.a
libircclient_a_SOURCES = libircclient.c
INCLUDES = -I$(top_srcdir)/src/irc/libircclient/include
AM_CFLAGS = -DIN_BUILDING_LIBIRC -DENABLE_THREADS -D_REENTRANT
endif

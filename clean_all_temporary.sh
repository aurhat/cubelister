#!/bin/bash

test "${PWD##*/}" = "csl" || exit 1

make distclean >/dev/null 2>&1

find . -name "*~" -exec rm -f {} \;
find . -name "*.bak" -exec rm -f {} \;
find . -name "*.orig" -exec rm -f {} \;
find . -name "Makefile.in*" -exec rm -f {} \;
find . -name "Makefile" -exec rm -f {} \;
find . -name "gmon.out" -exec rm -f {} \;

DIRS=(.)
for i in ${DIRS[*]}
do
 if test -d "$i"; then
  pushd $i >/dev/null
  rm -Rf \
   aclocal.m4      \
   autom4te.cache  \
   configure       \
   config.log      \
   config.sub      \
   config.status   \
   config.guess    \
   configure.in    \
   doc             \
   Doxyfile        \
   ipch            \
   debug*          \
   release*        \
   depcomp         \
   libtool         \
   ltmain.sh       \
   install-sh      \
   missing         \
   mkinstalldirs   \
   intltool-*      \
   po/LINGUAS      \
   po-plugin*      \
   Makefile.am     \
   po-engine/LINGUAS       \
   po-engine/Makevars      \
   src/config.h.in         \
   src/plugins/Makefile.am \

   popd >/dev/null
  fi
done

exit 0

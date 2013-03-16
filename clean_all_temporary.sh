#!/bin/bash

test "${PWD##*/}" = "csl" || exit 1

make distclean >/dev/null 2>&1

find . -iname "*~" -exec rm -f {} \;
find . -iname "*.bak" -exec rm -f {} \;
find . -iname "*.orig" -exec rm -f {} \;
find . -iname "Makefile.in" -exec rm -f {} \;
find . -iname "Makefile" -exec rm -f {} \;
find . -iname "gmon.out" -exec rm -f {} \;

DIRS=(.)
for i in ${DIRS[*]}
do
 if test -d "$i"; then
  pushd $i >/dev/null
  rm -Rf \
   doc             \
   Doxyfile        \
   aclocal.m4      \
   autom4te.cache  \
   configure       \
   config.sub      \
   config.status   \
   config.guess    \
   configure.in    \
   ipch            \
   debug*          \
   release*        \
   depcomp         \
   ltmain.sh       \
   install-sh      \
   missing         \
   mkinstalldirs   \
   intltool-*      \
   po/Makefile.in.in       \
   src/config.h.in         \
   src/plugins/Makefile.am \

   popd >/dev/null
  fi
done

exit 0

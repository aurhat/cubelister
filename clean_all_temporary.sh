#!/bin/sh

test "${PWD##*/}" = "csl" || exit 1

make distclean >/dev/null 2>&1

find . -iname "*~" -exec rm -f {} \;
find . -iname "*.bak" -exec rm -f {} \;
find . -iname "Makefile.in" -exec rm -f {} \;
find . -iname "Makefile" -exec rm -f {} \;
find . -iname "*.plg" -exec rm -f {} \;
find . -iname "*.ncb" -exec rm -f {} \;
find . -iname "*.kdevelop.pcs" -exec rm -f {} \;
find . -iname "*.kdevses" -exec rm -f {} \;
find . -iname "gmon.out" -exec rm -f {} \;
find . -type d -iname "debug" -exec rm -Rf {} \;
find . -type d -iname "release" -exec rm -Rf {} \;
find . -type d -iname "release-static" -exec rm -Rf {} \;
find . -type d -iname "release-static-engine" -exec rm -Rf {} \;

DIRS=(. src/enet)
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
   config.guess    \
   config.h.in     \
   debug*          \
   optimized*      \
   depcomp         \
   ltmain.sh       \
   install-sh      \
   missing         \
   mkinstalldirs   \
   intltool-*      \
   po/Makefile.in.in
  
   popd >/dev/null
  fi
done

exit 0

#!/bin/bash

fix_xpm()
{
 sed 's/static char \*/static const char \*/g' "$1" >"${1}.new" && \
  mv "${1}.new" "$1"
}

if test -z "$1"; then
 find . -iname "*.xpm" |while read line
 do
  fix_xpm $line
 done
else
 while test -n "$1"; do
  fix_xpm $1
  shift
 done
fi

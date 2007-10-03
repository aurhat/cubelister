#!/bin/bash

find . -iname "*.xpm" |while read line
do
 sed 's/static char \*/static const char \*/g' "$line" >"${line}.new" && \
  mv "${line}.new" "$line"
done

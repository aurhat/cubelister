#!/bin/bash

function find_plugins
{
  declare -r PLUGIN_DIR="src/plugins"
  declare -a plugins
  declare -a plugins_dirs

  for p in `find $PLUGIN_DIR -maxdepth 1 -type d | sort`
  do
    test "$p" == $PLUGIN_DIR -o ! -f "$p/Makefile.am" -o -f "$p/disabled" && continue
    p=$(basename "$p")
    test -z "$p" -o "${p:0:1}" == '.' && continue
    plugins[${#plugins[@]}]="$p"
    plugins_dirs[${#plugins_dirs[@]}]="${PLUGIN_DIR}/$p/Makefile"
  done

  local plugins=${plugins[@]}
  local plugins_dirs=${plugins_dirs[@]}

  ## src/plugins/Makefile.am
  sed s,"#SUBDIRS\s*=.*","SUBDIRS = ${plugins}", $PLUGIN_DIR/Makefile.am.in >$PLUGIN_DIR/Makefile.am
  ## configure.in
  sed s,"PLUGINS\s*=.*","PLUGINS=\"${plugins}\"","; \
     "s,"PLUGINS_MAKEFILES","${plugins_dirs}", configure.in.in >configure.in
}

function run_autotools
{
  aclocal
  libtoolize --automake --force --copy
  autoheader
  intltoolize --force --copy
  automake -a -c
  autoconf
}

### main() ###

cd "${0%/*}"

find_plugins
run_autotools

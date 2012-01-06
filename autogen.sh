#!/bin/bash

function find_plugins
{
  declare -r PLUGIN_DIR="src/plugins"
  local plugins
  local plugins_stat
  local plugins_dirs

  declare -i i=0
  declare -i cfgstatlen=0
  local space=""
  local cfgstat=`grep "Install prefix..." configure.in.template`

  ## calc length of the config status lines
  if test -n "$cfgstat"; then
    cfgstat=`echo -n "${cfgstat#*\"}"`
    cfgstatlen=`echo -n "${cfgstat%:*}" | wc -c`+2
  fi

  for p in `find $PLUGIN_DIR -maxdepth 1 -type d | sort`
  do
    test "$p" == $PLUGIN_DIR -o ! -f "${p}/Makefile.am" -o -f "${p}/disabled" && continue
    p=$(basename "$p")
    test -z "$p" -o "${p:0:1}" == '.' && continue

    test $i -gt 0 && space="\n"`printf "%${cfgstatlen}s" ' '`

    plugins+=" ${p}"
    plugins_stat+="${space}${p}"
    plugins_dirs+=" ${PLUGIN_DIR}/${p}/Makefile"
    i=i+1
  done

  ## src/plugins/Makefile.am
  sed s,"#SUBDIRS\s*=.*","SUBDIRS =${plugins}", $PLUGIN_DIR/Makefile.am.template >$PLUGIN_DIR/Makefile.am
  ## configure.in
  sed  s,"PLUGINS_ENABLED","${plugins_stat}","; \
      "s,"PLUGINS_MAKEFILES","${plugins_dirs}", configure.in.template >configure.in
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

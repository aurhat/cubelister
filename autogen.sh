#!/usr/bin/env bash

set -e

function get_output_indent
{
    declare -i i=0
    declare -i len=0
    local line=`grep "Install prefix..." configure.ac.template`

    if [ -n "$line" ]; then
        line=`echo -n "${line#*\"}"`
        len=`echo -n "${line%:*}" | wc -c`+2
    fi

    OUTPUT_STATUS_INDENT=`printf "%${len}s" ' '`
}

function get_plugins
{
    local p
    local indent
    local plug_name
    local plugins_enabled
    local plugins_makefiles
    local plugins_podir
    local plugins_intltool
    declare -a plugins
    declare -r PLUGIN_DIR="src/plugins"

    for p in `find $PLUGIN_DIR -maxdepth 1 -type d | sort`
    do
        test $p == $PLUGIN_DIR -o ! -f "${p}/Makefile.am" -o -f "${p}/disabled" && continue

        plug_name=`basename $p`
        plugins+=($plug_name)
        plugins_enabled+="${indent}${plug_name}"
        plugins_makefiles+="${p}/Makefile \\\\\n\t  "

        if [ -f "${p}/po/POTFILES.in" ]; then
            local podir="po-plugin-${plug_name}"
            plugins_podir+="${podir} "
            plugins_intltool+="IT_PO_SUBDIR([${podir}])\n"
            plugins_makefiles+="${podir}/Makefile.in \\\\\n\t  "
            PO_PLUGINS+=($p)
        fi

        test -z $indent && indent="\n$OUTPUT_STATUS_INDENT"
    done

    sed s,"PO_PLUGINS","${plugins_podir}", \
        Makefile.am.template >Makefile.am

    sed s,"#SUBDIRS\s*=.*","SUBDIRS = ${plugins[*]}", \
        $PLUGIN_DIR/Makefile.am.template >$PLUGIN_DIR/Makefile.am

    sed -e s,"PLUGINS_INTLTOOL","${plugins_intltool}",\
        -e s,"PLUGINS_MAKEFILES","${plugins_makefiles}",\
        -e s,"PLUGINS_ENABLED","${plugins_enabled}",\
        configure.ac.template >configure.ac
}

function get_autotools
{
    local t tool
    declare -a tools="ACLOCAL AUTOCONF AUTOHEADER AUTOMAKE INTLTOOLIZE LIBTOOLIZE"

    for t in ${tools[@]}
    do
      if [ -n "${!t}" ]; then
          tool=${!t}
      else
          tool=`echo $t | tr '[A-Z]' '[a-z]'`
      fi
      if [ -z `which $tool` &> /dev/null ]; then
          echo "Error: $tool not found." >&2
          exit 1
      fi
      export $t=$tool
    done
}

function run_autotools
{
    $ACLOCAL --force $VERBOSE
    $LIBTOOLIZE $DEBUG --automake --force $AUTOTOOLSCOPY
    $AUTOHEADER --force $DEBUG $VERBOSE
    $INTLTOOLIZE --force $DEBUG $AUTOTOOLSCOPY
    update_po_dirs
    $AUTOMAKE --gnu --add-missing $VERBOSE $AUTOTOOLSCOPY
    $AUTOCONF $DEBUG $VERBOSE
}

function update_po_dir
{
    local l linguas
    local domain=${1:+"-${1}"}
    local path="po${domain}"

    pushd $path >/dev/null

    for l in `find -name "*.po" |sort`
    do
        l=${l##*/}; l=${l%.po}
        linguas="${linguas}${l}\n"
    done

    if [ -n "$domain" ]; then
        $COPYTOOL ../po/Makefile.in.in .
        test ! -r Makevars && $COPYTOOL ../po/Makevars .
    fi

    sed -e '/^\(GETTEXT_PACKAGE\|subdir\) =/s/[    ]*$/'$domain'/' \
        -i Makefile.in.in

    echo -en $linguas >LINGUAS

    popd >/dev/null
}

function update_po_dirs
{
   update_po_dir
   update_po_dir "engine"

   for p in "${PO_PLUGINS[@]}"
   do
       local plug_name="${p##*/}"
       local domain="plugin-${plug_name}"
       local po_dir="po-${domain}"

       test -d $po_dir || mkdir $po_dir
       pushd $po_dir >/dev/null
       $COPYTOOL ../${p}/po/* .
       popd >/dev/null

       update_po_dir $domain
   done
}

function show_help
{
    echo
    echo "autogen.sh usage:"
    echo
    echo "  Produces all files necessary to build the the project."
    echo "  The files are sym-linked by default, if you run ./autogen.sh without an option."
    echo
    echo "    -h        Print this help message."
    echo "    -c        Copy files instead to sym-link them."
    echo "    -f FILE   Output everything to FILE (debugging). Useful for debug output."
    echo "    -v        Be more verbose about every step (debugging)."
    echo
    echo "  You can overwrite the automatically determined location of aclocal (>= 1.8),"
    echo "  automake (>= 1.8), autoheader, autoconf, libtoolize and intltoolize using:"
    echo
    echo "    ACLOCAL=/opt/foo/bin/aclocal-1.9 AUTOMAKE=automake-1.9 ./autogen.sh"
    echo
}

### main() ###

cd "${0%/*}"

COPYTOOL="ln -s -f"

while getopts "chvf:" options
do
    case "$options" in
        h)
            show_help
            exit 0
            ;;
        c)
            COPYTOOL="cp -p -f"
            AUTOTOOLSCOPY="-c"
            ;;
        f)
            OUTPUTFILE=$OPTARG
            ;;
        v)
            DEBUG="--debug"
            VERBOSE="--verbose"
            set -x
            ;;
    esac
done

if [ -n "$OUTPUTFILE" ]; then
    exec &>$OUTPUTFILE
fi

get_autotools
get_output_indent
get_plugins
run_autotools

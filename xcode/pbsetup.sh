#! /bin/sh
#
# Usage: $0 SRCROOT SYMROOT
#
# Display errors/warnings in the correct format for ProjectBuilder parsing
#
if [ $# -ne 2 ] ; then
    echo "${0}:${LINENO}: error: Usage: ${0} SRCROOT SYMROOT"
    exit 1
fi
#
# Copy config.h.mac to config,h if it doesn't exist
#
if [ ! -f "${1}/../src/irc/libircclient/include/config.h" ]; then
    cp "${1}/../src/irc/libircclient/include/config.h.mac" "${1}/../src/irc/libircclient/include/config.h"
    if [ ! -f "${1}/../src/irc/libircclient/include/config.h" ]; then
        echo "${0}:${LINENO}: error: unable to create setup file \"${1}/../src/irc/libircclient/include/config.h\""
        exit 1
    fi
fi

/***************************************************************************
 *   Copyright (C) 2007-2013 by Glen Masgai                                *
 *   mimosius@users.sourceforge.net                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef _MSC_VER
#if defined(HAVE_CONFIG_H) || defined(__WXMAC_XCODE__)
#include "config.h"
#endif
#else
#include <stdio.h>   // SEEK_SET, SEEK_CUR, SEEK_END
#include <io.h>      // _lseek

typedef int ssize_t;
#define lseek _lseek

int pread(unsigned int fd, char *buf, size_t count, int offset)
{
    if (_lseek(fd, offset, SEEK_SET)!=offset)
        return -1;

    return read(fd, buf, count);
}
#endif //_MSC_VER

#ifdef PACKAGE_VERSION
#undef PACKAGE_VERSION
#endif
#define PACKAGE_VERSION  "1.4.8"

#include "GeoIP.c"
#include "GeoIPCity.c"

/**********************************************************************
 * $Id: version.h.vc 2141 2008-07-26 20:47:29Z mloskot $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2007 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * This is a version header dedicated for use with Microsoft Visual C++
 * compiler.
 * NOTE: Remember to update this file manually when version
 * number changes.
 *
 **********************************************************************/
#ifndef GEOS_VERSION_H_INCLUDED
#define GEOS_VERSION_H_INCLUDED

#ifndef _MSC_VER
#error "This version.h intended for use with MS Visual C++"
#endif

#ifndef GEOS_VERSION_MAJOR
#define GEOS_VERSION_MAJOR 3
#endif

#ifndef GEOS_VERSION_MINOR
#define GEOS_VERSION_MINOR 1
#endif

#ifndef GEOS_VERSION_PATCH
#define GEOS_VERSION_PATCH 0
#endif

#ifndef GEOS_VERSION
#define GEOS_VERSION "3.1.0"
#endif

#ifndef GEOS_JTS_PORT
#define GEOS_JTS_PORT "1.7.1"
#endif

#endif // GEOS_VERSION_H_INCLUDED

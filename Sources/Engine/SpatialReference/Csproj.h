/***************************************************************
ILWIS integrates image, vector and thematic data in one unique 
and powerful package on the desktop. ILWIS delivers a wide 
range of feautures including import/export, digitizing, editing, 
analysis and display of data as well as production of 
quality mapsinformation about the sensor mounting platform

Exclusive rights of use by 52�North Initiative for Geospatial 
Open Source Software GmbH 2007, Germany

Copyright (C) 2007 by 52�North Initiative for Geospatial
Open Source Software GmbH

Author: Jan Hendrikse, Willem Nieuwenhuis,Wim Koolhoven 
Bas Restsios, Martin Schouwenburg, Lichun Wang, Jelle Wind 

Contact: Martin Schouwenburg; schouwenburg@itc.nl; 
tel +31-534874371

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (see gnu-gpl v2.txt); if not, write to
the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA or visit the web page of the Free
Software Foundation, http://www.fsf.org.

Created on: 2007-02-8
***************************************************************/
/* CoordSystemProjection
Copyright Ilwis System Development ITC
april 1998, by Wim Koolhoven
Last change:  WK    8 Apr 98    6:04 pm
*/

#ifndef ILWCSPROJ_H
#define ILWCSPROJ_H
#include "Engine\SpatialReference\csviall.h"
#include "Engine\SpatialReference\prj.h"
#include "Engine\Scripting\Instrucs.h"

class CoordSystemProjection: public CoordSystemViaLatLon
{
	friend class _export CoordSysProjectionForm;
public:
	CoordSystemProjection(const FileName&);
	_export CoordSystemProjection(const FileName&,int);
	~CoordSystemProjection();
	virtual void Store();
	virtual String sType() const;
	virtual bool fLatLon2Coord() const;
	virtual bool fCoord2LatLon() const;
	virtual LatLon llConv(const Coord&) const;
	virtual Coord cConv(const LatLon&) const;
	virtual bool _export fEqual(const IlwisObjectPtr& obj) const;
	String getIdentification(bool wkt=false);
	Projection prj;
};


#endif // ILWCSPROJ_H




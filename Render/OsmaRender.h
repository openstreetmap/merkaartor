//
// C++ Interface: ImportNMEA
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef OSMARENDER_H
#define OSMARENDER_H

#include "Map/Coord.h"
#include "Preferences/MerkaartorPreferences.h"

class MapDocument;
class QWidget;

class OsmaRender
{
public:
	OsmaRender(void);
public:
	virtual ~OsmaRender(void);

	void render(QWidget* aParent, MapDocument *aDoc, const CoordBox& aCoordBox = WORLD_COORDBOX);
};

#endif

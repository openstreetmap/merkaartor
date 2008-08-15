#ifndef OSMMAPADAPTER_H
#define OSMMAPADAPTER_H

#include "tilemapadapter.h"
//! MapAdapter for OpenStreetMap
/*!
 * This is a conveniece class, which extends and configures a TileMapAdapter
 *	@author Kai Winter <kaiwinter@gmx.de>
*/
class OSMMapAdapter : public TileMapAdapter
{
	Q_OBJECT
	public:
		//! constructor
		/*! 
		 * This construct a OpenStreetmap Adapter
		 */
		OSMMapAdapter();
		virtual ~OSMMapAdapter();
};

#endif

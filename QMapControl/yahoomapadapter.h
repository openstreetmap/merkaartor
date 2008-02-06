#ifndef YAHOOMAPADAPTER_H
#define YAHOOMAPADAPTER_H

#include "tilemapadapter.h"
//! MapAdapter for Yahoo Maps
/*!
 *	@author Kai Winter <kaiwinter@gmx.de>
*/
class YahooMapAdapter : public TileMapAdapter
{
	Q_OBJECT
	public:
		//! constructor
		/*! 
	 	 * This construct a Yahoo Adapter
		 */
		YahooMapAdapter();
		YahooMapAdapter(QString host, QString url);
		virtual ~YahooMapAdapter();
		bool isValid(int x, int y, int z) const;
	protected:
		virtual int tilesonzoomlevel(int zoomlevel) const;
		virtual int getyoffset(int y) const;
};

#endif

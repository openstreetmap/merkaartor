#ifndef GEOMETRYLAYER_H
#define GEOMETRYLAYER_H

#include "layer.h"

//! GeometryLayer class
/*!
 * There are two different layer types:
 *  - MapLayer: Displays Maps, but also Geometries. The configuration for displaying maps have to be done in the MapAdapter
 *  - GeometryLayer: Only displays Geometry objects.
 * 
 * MapLayers also can display Geometry objects. The difference to the GeometryLayer is the repainting. Objects that are
 * added to a MapLayer are "baken" on the map. This means, when you change it´s position for example the changes are
 * not visible until a new offscreen image has been drawn. If you have "static" Geometries which won´t change their
 * position this is fine. But if you want to change the objects position or pen you should use a GeometryLayer. Those
 * are repainted immediately on changes.
 * 
 *	@author Kai Winter <kaiwinter@gmx.de>
 */
class GeometryLayer : public Layer
{
	Q_OBJECT
	public:
	//! GeometryLayer constructor
	/*!
	 * This is used to construct a map layer.
	 * 
	 * @param layername The name of the Layer
	 * @param mapadapter The MapAdapter which does coordinate translation and Query-String-Forming
	 * @param takeevents Should the Layer receive MouseEvents? This is set to true by default. Setting it to false could
	 * be something like a "speed up hint"
	 */
		GeometryLayer(QString layername, MapAdapter* mapadapter, bool takeevents=true);
		virtual ~GeometryLayer();

};

#endif

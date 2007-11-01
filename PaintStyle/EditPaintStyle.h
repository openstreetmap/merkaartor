#ifndef MERKAARTOR_EDITPAINTSTYLE_H_
#define MERKAARTOR_EDITPAINTSTYLE_H_

#include "PaintStyle.h"

class EditPaintStylePrivate;
class Projection;
class QPainter;


class EditPaintStyle : public PaintStyle
{
	public:
		EditPaintStyle(QPainter& P, const Projection& theProjection);
		virtual ~EditPaintStyle();
		
	private:
		EditPaintStylePrivate* p;
};

#endif
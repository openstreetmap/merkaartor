#ifndef MERKAARTOR_EDITPAINTSTYLE_H_
#define MERKAARTOR_EDITPAINTSTYLE_H_

#include "PaintStyle.h"

class EditPaintStylePrivate;
class Projection;
class QPainter;

#include <vector>

class EditPaintStyle : public PaintStyle
{
	public:
		static std::vector<FeaturePainter> Painters;
	public:
		EditPaintStyle(QPainter& P, const Projection& theProjection);
		virtual ~EditPaintStyle();
		
	private:
		EditPaintStylePrivate* p;
};

#endif

#ifndef MERKAARTOR_EDITPAINTSTYLE_H_
#define MERKAARTOR_EDITPAINTSTYLE_H_

#include "PaintStyle.h"

class EditPaintStylePrivate;
class Projection;
class QPainter;
class QString;

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

/* FEATUREPAINTSELECTOR */

class EPBackgroundLayer : public PaintStyleLayer
{
	public:
		void setP(EditPaintStylePrivate* p);
		virtual void draw(Road* R);
		virtual void draw(TrackPoint* Pt);
		virtual void draw(Relation* R);
	private:
		EditPaintStylePrivate* p;
};

class EPForegroundLayer : public PaintStyleLayer
{
	public:
		void setP(EditPaintStylePrivate* p);
		virtual void draw(Road* R);
		virtual void draw(TrackPoint* Pt);
		virtual void draw(Relation* R);
	private:
		EditPaintStylePrivate* p;
};

class EPTouchupLayer : public PaintStyleLayer
{
	public:
		void setP(EditPaintStylePrivate* p);
		virtual void draw(Road* R);
		virtual void draw(TrackPoint* Pt);
		virtual void draw(Relation* R);
	private:
		EditPaintStylePrivate* p;
};

class EPLabelLayer : public PaintStyleLayer
{
	public:
		void setP(EditPaintStylePrivate* p);
		virtual void draw(Road* R);
		virtual void draw(TrackPoint* Pt);
		virtual void draw(Relation* R);
	private:
		EditPaintStylePrivate* p;
};

void savePainters(const QString& filename);
void loadPainters(const QString& filename);

#endif

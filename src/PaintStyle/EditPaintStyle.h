#ifndef MERKAARTOR_EDITPAINTSTYLE_H_
#define MERKAARTOR_EDITPAINTSTYLE_H_

#include "PaintStyle.h"

class EditPaintStylePrivate;
class Projection;
class QPainter;
class QString;

#include <QList>

#define M_STYLE EditPaintStyle::instance()

class EditPaintStyle : public PaintStyle
{
	public:
		static EditPaintStyle* instance() {
			if (!m_EPSInstance) {
				m_EPSInstance = new EditPaintStyle;
			}

			return m_EPSInstance;
		}

		EditPaintStyle();
		virtual ~EditPaintStyle();
		void initialize(QPainter& P, const Projection& theProjection);

		int painterSize();
		const GlobalPainter& getGlobalPainter() const;
		void setGlobalPainter(GlobalPainter aGlobalPainter);
		const FeaturePainter* getPainter(int i) const;
		QList<FeaturePainter> getPainters() const;
		void setPainters(QList<FeaturePainter> aPainters);

		void savePainters(const QString& filename);
		void loadPainters(const QString& filename);

	private:
		EditPaintStylePrivate* p;
		QList<FeaturePainter> Painters;
		GlobalPainter globalPainter;
		
		static EditPaintStyle* m_EPSInstance;
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

#endif

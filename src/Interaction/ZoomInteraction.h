#ifndef MERKATOR_ZOOMINTERACTION_H_
#define MERKATOR_ZOOMINTERACTION_H_

#include "Interaction/Interaction.h"

#include <QtCore/QPointF>

class MapView;

class ZoomInteraction : public Interaction
{
	public:
		ZoomInteraction(MapView* aView);
		~ZoomInteraction();

		virtual void mouseReleaseEvent(QMouseEvent * event);
		virtual void mouseMoveEvent(QMouseEvent* event);
		virtual void paintEvent(QPaintEvent* anEvent, QPainter& thePainter);
#ifndef Q_OS_SYMBIAN
		virtual QCursor cursor() const;
#endif

	private:
		bool HaveFirstPoint;
		QPointF P1, P2;
};

#endif



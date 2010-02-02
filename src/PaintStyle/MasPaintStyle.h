#ifndef MERKAARTOR_EDITPAINTSTYLE_H_
#define MERKAARTOR_EDITPAINTSTYLE_H_

#include "PaintStyle.h"

class MapView;
class PaintStylePrivate;

#include <QList>

#define M_STYLE MasPaintStyle::instance()

class MasPaintStyle
{
	public:
		static MasPaintStyle* instance() {
			if (!m_EPSInstance) {
				m_EPSInstance = new MasPaintStyle;
			}

			return m_EPSInstance;
		}

		MasPaintStyle();
		virtual ~MasPaintStyle();
		void initialize(QPainter& P, MapView& theView);

		int painterSize();
		const GlobalPainter& getGlobalPainter() const;
		void setGlobalPainter(GlobalPainter aGlobalPainter);
		const FeaturePainter* getPainter(int i) const;
		QList<FeaturePainter> getPainters() const;
		void setPainters(QList<FeaturePainter> aPainters);

		void savePainters(const QString& filename);
		void loadPainters(const QString& filename);

	private:
		QList<FeaturePainter> Painters;
		GlobalPainter globalPainter;

		static MasPaintStyle* m_EPSInstance;
};

#endif

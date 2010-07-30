#ifndef MERKAARTOR_MAPCSSPAINTSTYLE_H_
#define MERKAARTOR_MAPCSSPAINTSTYLE_H_

#include "Painter.h"

class MapView;
class PaintStylePrivate;

#include <QList>

class MapCSSPaintstyle
{
    public:
        static MapCSSPaintstyle* instance() {
            if (!m_MapCSSInstance) {
                m_MapCSSInstance = new MapCSSPaintstyle;
            }

            return m_MapCSSInstance;
        }

        MapCSSPaintstyle();
        virtual ~MapCSSPaintstyle();
        void initialize(QPainter& P, MapView& theView);

        int painterSize();
        const GlobalPainter& getGlobalPainter() const;
        void setGlobalPainter(GlobalPainter aGlobalPainter);
        const Painter* getPainter(int i) const;
        QList<Painter> getPainters() const;
        void setPainters(QList<Painter> aPainters);

        void savePainters(const QString& filename);
        void loadPainters(const QString& filename);

    private:
        QList<Painter> Painters;
        GlobalPainter globalPainter;

        static MapCSSPaintstyle* m_MapCSSInstance;
};

#endif

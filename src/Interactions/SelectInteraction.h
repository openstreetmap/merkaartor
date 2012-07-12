#ifndef MERKAARTOR_SELECTINTERACTION_H_
#define MERKAARTOR_SELECTINTERACTION_H_

#include "Interaction.h"

class SelectInteraction : public FeatureSnapInteraction
{
    Q_OBJECT

public:
    SelectInteraction();
    ~SelectInteraction(void);

    virtual void paintEvent(QPaintEvent* anEvent, QPainter& thePainter);
    virtual void snapMousePressEvent(QMouseEvent * event, Feature* aLast);
    virtual void snapMouseReleaseEvent(QMouseEvent * event, Feature* aLast);
    virtual void snapMouseMoveEvent(QMouseEvent* event, Feature* aLast);
    virtual void snapMouseDoubleClickEvent(QMouseEvent* event, Feature* aLast);
    virtual QString toHtml();
#ifndef _MOBILE
    virtual QCursor cursor() const;
#endif

    virtual bool isIdle() const;

signals:
    void setSelection(Feature* aFeature);
    void setMultiSelection(const QList<Feature*>& aFeatureList);
    void toggleSelection(Feature* aFeature);
    void addSelection(Feature* aFeature);

protected:
    bool Dragging;
    Coord StartDrag;
    Coord EndDrag;

    QCursor defaultCursor;
};

#endif



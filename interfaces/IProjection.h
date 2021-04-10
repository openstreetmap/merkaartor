#ifndef MERKATOR_IPROJECTION_H_
#define MERKATOR_IPROJECTION_H_

class QString;
class QPointF;

class IProjection
{
public:
    virtual ~IProjection(void) {};

    virtual QPointF project(const QPointF& pt) const = 0;
    virtual QPointF inverse(const QPointF& pt) const = 0;

    virtual QString getProjectionType() const = 0;
};


#endif



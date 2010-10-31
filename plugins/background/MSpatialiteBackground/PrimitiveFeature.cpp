#include "PrimitiveFeature.h"

#include <PrimitivePainter.h>

PrimitiveFeature::PrimitiveFeature()
    : type(0)
{
}

int PrimitiveFeature::tagSize() const
{
    return Tags.size();
}

QString PrimitiveFeature::tagValue(int i) const
{
    return Tags[i].second;
}

QString PrimitiveFeature::tagKey(int i) const
{
    return Tags[i].first;
}

int PrimitiveFeature::findKey(const QString &k) const
{
    for (int i=0; i<Tags.size(); ++i)
        if (Tags[i].first == k)
            return i;
    return Tags.size();
}

QString PrimitiveFeature::tagValue(const QString& k, const QString& Default) const
{
    for (int i=0; i<Tags.size(); ++i)
        if (Tags[i].first == k)
            return Tags[i].second;
    return Default;
}

void PrimitiveFeature::setType(quint32 aTyp)
{
    type = aTyp;
}

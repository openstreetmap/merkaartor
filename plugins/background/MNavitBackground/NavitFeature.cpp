#include "NavitFeature.h"

#include <PrimitivePainter.h>

NavitFeature::NavitFeature()
    : type(0)
{
}

int NavitFeature::tagSize() const
{
    return Tags.size();
}

QString NavitFeature::tagValue(int i) const
{
    return Tags[i].second;
}

QString NavitFeature::tagKey(int i) const
{
    return Tags[i].first;
}

int NavitFeature::findKey(const QString &k) const
{
    for (int i=0; i<Tags.size(); ++i)
        if (Tags[i].first == k)
            return i;
    return Tags.size();
}

QString NavitFeature::tagValue(const QString& k, const QString& Default) const
{
    for (int i=0; i<Tags.size(); ++i)
        if (Tags[i].first == k)
            return Tags[i].second;
    return Default;
}

void NavitFeature::setType(quint32 aTyp)
{
    type = aTyp;
}

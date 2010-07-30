#ifndef IFEATURE_H
#define IFEATURE_H

#include <QString>
#include <QDateTime>

class IFeature
{
public:
    typedef enum {
        Point				= 0x00000000,
        LineString			= 0x00000001,
        Polygon             = 0x00000002,
        OsmSegment			= 0x10000000,
        OsmRelation			= 0x10000001,
        All					= 0xffffffff
    } FeatureType;

public:
    virtual FeatureType getType() const = 0;

    virtual QString xmlId() const = 0;
    virtual const QDateTime& time() const = 0;
    virtual int versionNumber() const = 0;
    virtual const QString& user() const = 0;

    virtual int sizeParents() const = 0;
    virtual IFeature* getParent(int i) = 0;
    virtual const IFeature* getParent(int i) const = 0;

    virtual bool hasPainter(double PixelPerM) const = 0;

    /** check if the feature is logically deleted
     * @return true if logically deleted
     */
    virtual bool isDeleted() const = 0;

    /** @return the number of tags for the current object
         */
    virtual int tagSize() const = 0;

    /** if a tag with the key "k" exists, return its index.
         * if the key doesn't exist, return the number of tags
         * @return index of tag
         */
    virtual int findKey(const QString& k) const = 0;

    /** return the value of the tag at the position "i".
         * position start at 0.
         * Be carefull: no verification is made on i.
         * @return the value
         */
    virtual QString tagValue(int i) const = 0;

    /** return the value of the tag with the key "k".
         * if such a tag doesn't exists, return Default.
         * @return value or Default
         */
    virtual QString tagValue(const QString& k, const QString& Default) const = 0;

    /** return the value of the tag at the position "i".
         * position start at 0.
         * Be carefull: no verification is made on i.
         * @return the value
        */
    virtual QString tagKey(int i) const = 0;

};

#endif // IFEATURE_H

#ifndef IFEATURE_H
#define IFEATURE_H

#include <QString>
#include <QDateTime>
#include <QPair>
#include <QMetaType>

class IFeature
{
public:
    typedef enum {
        Uninitialized       = 0x00,
        Point				= 0x01,
        LineString			= 0x02,
        Polygon             = 0x04,
        OsmRelation			= 0x10,
        GpxSegment			= 0x20,
        Conflict            = 0x40,
        Special             = 0x80,
        All					= 0xff
    } FeatureType;

    struct FId {
        FId() : type(IFeature::Uninitialized), numId(0) {}
        FId(char a, qint64 b) : type(a), numId(b) {}

        bool operator==(const FId& R) const
        {
            return ((type == R.type) && (numId == R.numId));
        }
        char type;
        qint64 numId;
    };

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

    /** Give the id of the feature.
     *  If the feature has no id, a random id is generated
     * @return the id of the current feature
     */
    const IFeature::FId& id() const;

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


    /** check if the feature has been uploaded
     * @return true if uploaded
     */
    virtual bool isUploaded() const = 0;

    /** check if the dirty status of the feature
     * @return true if the feature is dirty
     */
    virtual bool isDirty() const = 0;

    /** check if the feature is visible
     * @return true if visible
     */
    virtual bool isVisible() = 0;

    /** check if the feature is read-only
     * @return true if is read-only
     */
    virtual bool isReadonly() = 0;

};

Q_DECLARE_METATYPE(IFeature::FId)

#endif // IFEATURE_H

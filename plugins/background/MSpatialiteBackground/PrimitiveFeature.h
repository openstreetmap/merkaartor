#ifndef PRIMITIVEFEATURE_H
#define PRIMITIVEFEATURE_H

#include "IFeature.h"
#include <QList>
#include <QVector>
#include <QPair>
#include <QPoint>
#include <QPainterPath>

class PrimitivePainter;

class PrimitiveAttribute
{
public:
    PrimitiveAttribute()
        : type(0) {}
    PrimitiveAttribute(qint32 theType, QByteArray theAttribute)
        : type(theType), attribute(theAttribute) {}

public:
    quint32 type;
    QByteArray attribute;
};


class PrimitiveFeature : public IFeature
{

public:
    PrimitiveFeature();

    virtual char getType() const { return IFeature::All; }

    virtual QString xmlId() const { return QString(); }
    virtual const QDateTime time() const { return QDateTime::currentDateTime(); }
    virtual int versionNumber() const { return -1; }
    virtual const QString& user() const { return QString(); }

    virtual int sizeParents() const { return 0; }
    virtual IFeature* getParent(int) { return NULL; }
    virtual const IFeature* getParent(int) const { return NULL; }

    virtual bool hasPainter(double) const { return false; }

    /** Give the id of the feature.
     *  If the feature has no id, a random id is generated
     * @return the id of the current feature
     */
    virtual const IFeature::FId& id() const {return theId;}

    /** check if the feature is logically deleted
     * @return true if logically deleted
     */
    virtual bool isDeleted() const { return false; }

    /** @return the number of tags for the current object
         */
    virtual int tagSize() const;

    /** if a tag with the key "k" exists, return its index.
         * if the key doesn't exist, return the number of tags
         * @return index of tag
         */
    virtual int findKey(const QString& k) const;

    /** return the value of the tag at the position "i".
         * position start at 0.
         * Be carefull: no verification is made on i.
         * @return the value
         */
    virtual QString tagValue(int i) const;

    /** return the value of the tag with the key "k".
         * if such a tag doesn't exists, return Default.
         * @return value or Default
         */
    virtual QString tagValue(const QString& k, const QString& Default) const;

    /** return the value of the tag at the position "i".
         * position start at 0.
         * Be carefull: no verification is made on i.
         * @return the value
        */
    virtual QString tagKey(int i) const;

    /** check if the feature has been uploaded
     * @return true if uploaded
     */
    virtual bool isUploaded() const { return false; }

    /** check if the dirty status of the feature
     * @return true if the feature is dirty
     */
    virtual bool isDirty() const { return false; }

    /** check if the feature is visible
     * @return true if visible
     */
    virtual bool isVisible() { return true; }

    /** check if the feature is read-only
     * @return true if is read-only
     */
    virtual bool isReadonly() { return true; }

    virtual const QPainterPath& getPath() const {return thePath;}

    void setType(quint32 aTyp);

public:
    quint32 type;
    quint16 order;
    QVector<QPoint> coordinates;
    QList<PrimitiveAttribute> attributes;

    IFeature::FId theId;
    QPainterPath thePath;
    QList<QPair<QString, QString> > Tags;
};

#endif // NAVITFEATURE_H

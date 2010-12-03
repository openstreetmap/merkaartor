#include "Features.h"
#include "Command.h"
#include "DocumentCommands.h"
#include "FeatureCommands.h"
#include "RelationCommands.h"
#include "WayCommands.h"
#include "NodeCommands.h"
#include "Document.h"
#include "Layer.h"
#include "PaintStyle/MasPaintStyle.h"
#include "Utils/TagSelector.h"
#include "MapView.h"
#include "PropertiesDock.h"

#include "Utils/Utils.h"

#include <QApplication>
#include <QUuid>
#include <QProgressDialog>
#include <QPainter>
#include <QPainterPath>

#include <algorithm>

qint64 g_rndId = 0;
IFeature::FId Feature::newId(IFeature::FeatureType type, Document* d) const
{
    IFeature::FId id = IFeature::FId(type, --g_rndId);
    if (d)
        while (d->getFeature(id))
            id = IFeature::FId(type, --g_rndId);

    return id;
}

//static QString randomId()
//{
//	QUuid uuid = QUuid::createUuid();
//#ifdef _MOBILE
//	return uuid.toString();
//#else
//	// This is fairly horrible, but it's also around 10 times faster than QUuid::toString()
//	// and randomId() is called a lot during large imports
//
//	// Lookup table of hex value-pairs representing a byte
//	static char hex[(2*256)+1] = "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f606162636465666768696a6b6c6d6e6f707172737475767778797a7b7c7d7e7f808182838485868788898a8b8c8d8e8f909192939495969798999a9b9c9d9e9fa0a1a2a3a4a5a6a7a8a9aaabacadaeafb0b1b2b3b4b5b6b7b8b9babbbcbdbebfc0c1c2c3c4c5c6c7c8c9cacbcccdcecfd0d1d2d3d4d5d6d7d8d9dadbdcdddedfe0e1e2e3e4e5e6e7e8e9eaebecedeeeff0f1f2f3f4f5f6f7f8f9fafbfcfdfeff";
//
//	char buffer[39] = "{________-____-____-____-____________}";
//
//
//
//	// {__%08x__-____-____-____-____________}
//	memcpy(&buffer[ 1], &hex[((uuid.data1 >> 24) & 0xFF)], 2);
//	memcpy(&buffer[ 3], &hex[((uuid.data1 >> 16) & 0xFF)], 2);
//	memcpy(&buffer[ 5], &hex[((uuid.data1 >>  8) & 0xFF)], 2);
//	memcpy(&buffer[ 7], &hex[((uuid.data1 >>  0) & 0xFF)], 2);
//
//	// {________-%04x-____-____-____________}
//	memcpy(&buffer[10], &hex[((uuid.data2 >>  8) & 0xFF)], 2);
//	memcpy(&buffer[12], &hex[((uuid.data2 >>  0) & 0xFF)], 2);
//
//	// {________-____-%04x-____-____________}
//	memcpy(&buffer[15], &hex[((uuid.data3 >>  8) & 0xFF)], 2);
//	memcpy(&buffer[17], &hex[((uuid.data3 >>  0) & 0xFF)], 2);
//
//	// {________-____-____-%04x-____________}
//	memcpy(&buffer[20], &hex[uuid.data4[0]], 2);
//	memcpy(&buffer[22], &hex[uuid.data4[1]], 2);
//
//	// {________-____-____-____-___%012x____}
//	for (int i=0; i<6; i++) {
//		memcpy(&buffer[25+(i*2)], &hex[uuid.data4[i+2]], 2);
//	}
//
//	return QString::fromAscii(buffer,38);
//#endif
//}

namespace boost
{
    void intrusive_ptr_add_ref(Feature * p)
    {
        ++(p->m_references);
    }
    void intrusive_ptr_release(Feature * p)
    {
        if (--(p->m_references) == 0) {
            if (p->layer())
                p->layer()->deleteFeature(p);
            else
                delete p;
        }
    }
} // namespace boost


void copyTags(Feature* Dest, Feature* Src)
{
    for (int i=0; i<Src->tagSize(); ++i)
        Dest->setTag(Src->tagKey(i),Src->tagValue(i));
}

class MapFeaturePrivate
{
    public:
        MapFeaturePrivate()
            :  LastActor(Feature::User),
                PossiblePaintersUpToDate(false),
                PixelPerMForPainter(-1), CurrentPainter(0), HasPainter(false),
                theFeature(0), LastPartNotification(0),
                Time(QDateTime::currentDateTime()), Deleted(false), Visible(true), Uploaded(false), ReadOnly(false), FilterRevision(-1)
                , Virtual(false), Special(false), DirtyLevel(0)
                , parentLayer(0)
        {
            initVersionNumber();
        }
        MapFeaturePrivate(const MapFeaturePrivate& other)
            : Tags(other.Tags), LastActor(other.LastActor),
                PossiblePaintersUpToDate(false),
                PixelPerMForPainter(-1), CurrentPainter(0), HasPainter(false),
                theFeature(0), LastPartNotification(0),
                Time(other.Time), Deleted(false), Visible(true), Uploaded(false), ReadOnly(false), FilterRevision(-1)
                , Virtual(other.Virtual), Special(other.Special), DirtyLevel(0)
                , parentLayer(0)
        {
            initVersionNumber();
        }

        void updatePossiblePainters();
        void blankPainters();
        void updatePainters(double PixelPerM);
        void initVersionNumber();

        mutable IFeature::FId Id;
        QList<QPair<quint32, quint32> > Tags;
        Feature::ActorType LastActor;
        QList<const FeaturePainter*> PossiblePainters;
        bool PossiblePaintersUpToDate;
        double PixelPerMForPainter;
        const FeaturePainter* CurrentPainter;
        bool HasPainter;
        Feature* theFeature;
        QList<Feature*> Parents;
        int LastPartNotification;
        QDateTime Time;
        QString User;
        int VersionNumber;
        bool Deleted;
        bool Visible;
        bool Uploaded;
        bool ReadOnly;
        int FilterRevision;
        RenderPriority theRenderPriority;
        bool Virtual;
        bool Special;
        int DirtyLevel;
        QList<FilterLayer*> FilterLayers;
        double Alpha;
        Layer* parentLayer;
};

void MapFeaturePrivate::initVersionNumber()
{
//    static int VN = -1;
//    VersionNumber = VN--;
    VersionNumber = 0;
}

Feature::Feature()
: p(0), MetaUpToDate(false), m_references(0)
{
     p = new MapFeaturePrivate;
     p->theFeature = this;
     p->Id = IFeature::FId(IFeature::Uninitialized, 0);

//     qDebug() << "Feature size: " << sizeof(Feature) << sizeof(MapFeaturePrivate);
}

Feature::Feature(const Feature& other)
: MetaUpToDate(false), m_references(0)
{
    p = new MapFeaturePrivate(*other.p);
    p->theFeature = this;
    p->Id = IFeature::FId(IFeature::Uninitialized, 0);
}

Feature::~Feature(void)
{
    // TODO Those cleanup shouldn't be necessary and lead to crashes
    //      Check for side effect of supressing them.
//    while (sizeParents())
//        getParent(0)->remove(this);
    delete p;
}

void Feature::setVersionNumber(int vn)
{
    p->VersionNumber = vn;
}

int Feature::versionNumber() const
{
    return p->VersionNumber;
}

void Feature::setLayer(Layer* aLayer)
{
    p->parentLayer = aLayer;
}

Layer* Feature::layer() const
{
    return p->parentLayer;
}

const RenderPriority& Feature::renderPriority()
{
    if (!MetaUpToDate)
        updateMeta();
    return p->theRenderPriority;
}

void Feature::setRenderPriority(const RenderPriority& aPriority)
{
    p->theRenderPriority = aPriority;
}

void Feature::setLastUpdated(Feature::ActorType A)
{
    p->LastActor = A;
}

Feature::ActorType Feature::lastUpdated() const
{
    Layer* L = p->parentLayer;
    if (L && L->classType() == Layer::DirtyLayerType)
        return Feature::User;
    else
        return p->LastActor;
}

QString Feature::stripToOSMId(const IFeature::FId& id)
{
    return QString::number(id.numId);
}

void Feature::setId(const IFeature::FId& id)
{
    if (id == p->Id)
        return;

    if (p->parentLayer)
    {
        if (p->Id.type != IFeature::Uninitialized)
            p->parentLayer->notifyIdUpdate(p->Id,0);
        if (id.type != IFeature::Uninitialized)
            p->parentLayer->notifyIdUpdate(id,this);
    }
    p->Id = id;
}

const IFeature::FId& Feature::resetId() const
{
    Layer* L = p->parentLayer;
    if (L) {
        p->Id = newId((IFeature::FeatureType)getType(), L->getDocument());
        L->notifyIdUpdate(p->Id,const_cast<Feature*>(this));
    } else
        p->Id = newId((IFeature::FeatureType)getType(), NULL);
    return p->Id;
}

const IFeature::FId& Feature::id() const
{
    if (p->Id.type == IFeature::Uninitialized)
        resetId();

    return p->Id;
}

QString Feature::xmlId() const
{
    return stripToOSMId(id());
}

bool Feature::hasOSMId() const
{
    return (p->Id.numId >= 0);
}

const QDateTime& Feature::time() const
{
    return p->Time;
}

void Feature::setTime(const QDateTime& time)
{
    p->Time = time;
}

const QString& Feature::user() const
{
    return p->User;
}

void Feature::setUser(const QString& user)
{
    p->User = user;
}

int Feature::incDirtyLevel(int inc)
{
    return p->DirtyLevel += inc;
}

int Feature::decDirtyLevel(int inc)
{
    return p->DirtyLevel -= inc;
}

int Feature::setDirtyLevel(int newLevel)
{
    return (p->DirtyLevel = newLevel);
}

int Feature::getDirtyLevel() const
{
    return p->DirtyLevel;
}

qreal Feature::getAlpha()
{
    if (!MetaUpToDate)
        updateMeta();
    return p->Alpha;
}

bool Feature::isDirty() const
{
    if (g_Merk_Frisius)
        return (p->DirtyLevel > 0);

    if (p->parentLayer)
        return (p->parentLayer->classType() == Layer::DirtyLayerType);
    else
        return false;
}

void Feature::setUploaded(bool state)
{
    p->Uploaded = state;
}

bool Feature::isUploaded() const
{
    return p->Uploaded;
}

bool Feature::isUploadable() const
{
    if (p->parentLayer)
        return (p->parentLayer->isUploadable());
    else
        return false;
}

void Feature::setReadonly(bool val)
{
    p->ReadOnly = val;
}

bool Feature::isReadonly()
{
    if (!MetaUpToDate)
        updateMeta();
    return p->ReadOnly;
}

void Feature::setDeleted(bool delState)
{
    if (delState == p->Deleted)
        return;

    if (layer()) {
        if (delState)
            layer()->indexRemove(boundingBox(false), this);
        else
            layer()->indexAdd(boundingBox(), this);
    }
    p->Deleted = delState;
}

bool Feature::isDeleted() const
{
    return p->Deleted;
}

void Feature::setVisible(bool state)
{
    if (state == p->Visible)
        return;
    p->Visible = state;
}

bool Feature::isVisible()
{
    if (!MetaUpToDate)
        updateMeta();
    return p->Visible;
}

bool Feature::isHidden()
{
    if (!MetaUpToDate)
        updateMeta();
    return !p->Visible;
}

void Feature::setVirtual(bool val)
{
    if (val == p->Virtual)
        return;

    p->Virtual = val;
    if (!p->Virtual)
        resetId();
}

bool Feature::isVirtual() const
{
    return p->Virtual;
}

void Feature::setSpecial(bool val)
{
    p->Special = val;
}

bool Feature::isSpecial() const
{
    return p->Special;
}

void Feature::setTag(int index, const QString& key, const QString& value)
{
    if (key.toLower() == "created_by")
        return;

    Q_ASSERT(p->parentLayer);
    Document* theDoc = p->parentLayer->getDocument();
    Q_ASSERT(theDoc);

    QPair<quint32, quint32> pi = theDoc->addToTagList(key, value);

    int i = 0;
    for (; i<p->Tags.size(); ++i)
        if (p->Tags[i].first == pi.first)
        {
            if (p->Tags[i].second == pi.second)
                return;
            theDoc->removeFromTagList(p->Tags[i].first, p->Tags[i].second);
            p->Tags[i].second = pi.second;
            break;
        }
    if (i == p->Tags.size()) {
        p->Tags.insert(p->Tags.begin() + index, pi);
    }
    invalidatePainter();
    invalidateMeta();
}

void Feature::setTag(const QString& key, const QString& value)
{
    if (key.toLower() == "created_by")
        return;

    Q_ASSERT(p->parentLayer);
    Document* theDoc = p->parentLayer->getDocument();
    Q_ASSERT(theDoc);

    QPair<quint32, quint32> pi = theDoc->addToTagList(key, value);

    int i = 0;
    for (; i<p->Tags.size(); ++i)
        if (p->Tags[i].first == pi.first)
        {
            if (p->Tags[i].second == pi.second)
                return;
            theDoc->removeFromTagList(p->Tags[i].first, p->Tags[i].second);
            p->Tags[i].second = pi.second;
            break;
        }
    if (i == p->Tags.size()) {
        p->Tags.push_back(pi);
    }
    invalidateMeta();
    invalidatePainter();
}

void Feature::clearTags()
{
    Q_ASSERT(p->parentLayer);
    Document* theDoc = p->parentLayer->getDocument();
    Q_ASSERT(theDoc);

    while (p->Tags.size()) {
        theDoc->removeFromTagList(p->Tags[0].first, p->Tags[0].second);
        p->Tags.erase(p->Tags.begin());
    }
    invalidateMeta();
    invalidatePainter();
}

void Feature::clearTag(const QString& k)
{
    Q_ASSERT(p->parentLayer);
    Document* theDoc = p->parentLayer->getDocument();
    Q_ASSERT(theDoc);

    quint32 ik = theDoc->getTagKeyIndex(k);

    for (int i=0; i<p->Tags.size(); ++i)
        if (p->Tags[i].first == ik)
        {
            theDoc->removeFromTagList(p->Tags[i].first, p->Tags[i].second);
            p->Tags.erase(p->Tags.begin()+i);
            break;
        }
    invalidateMeta();
    invalidatePainter();
}

void Feature::removeTag(int idx)
{
    Q_ASSERT(p->parentLayer);
    Document* theDoc = p->parentLayer->getDocument();
    Q_ASSERT(theDoc);

    theDoc->removeFromTagList(p->Tags[idx].first, p->Tags[idx].second);
    p->Tags.erase(p->Tags.begin()+idx);
    invalidateMeta();
    invalidatePainter();
}

int Feature::tagSize() const
{
    return p->Tags.size();
}

QString Feature::tagValue(int i) const
{
    Q_ASSERT(p->parentLayer);
    Document* theDoc = p->parentLayer->getDocument();
    Q_ASSERT(theDoc);

    return theDoc->getTagValue(p->Tags[i].second);
}

QString Feature::tagKey(int i) const
{
    Q_ASSERT(p->parentLayer);
    Document* theDoc = p->parentLayer->getDocument();
    Q_ASSERT(theDoc);

    return theDoc->getTagKey(p->Tags[i].first);
}

int Feature::findKey(const QString &k) const
{
    for (int i=0; i<p->Tags.size(); ++i)
        if (tagKey(i) == k)
            return i;
    return -1;
}

QString Feature::tagValue(const QString& k, const QString& Default) const
{
    for (int i=0; i<p->Tags.size(); ++i)
        if (tagKey(i) == k)
            return tagValue(i);
    return Default;
}

void Feature::invalidateMeta()
{
    MetaUpToDate = false;
}

void Feature::invalidatePainter()
{
    p->PossiblePaintersUpToDate = false;
    p->PixelPerMForPainter = -1;
}

const QPainterPath& Feature::getPath() const
{
    return QPainterPath();
}

void MapFeaturePrivate::updatePossiblePainters()
{
    //still match features with no tags and no parent, i.e. "lost" trackpoints
    if ( (theFeature->layer()->isTrack()) && M_PREFS->getDisableStyleForTracks() ) return blankPainters();

    if ( (theFeature->layer()->isTrack()) || theFeature->sizeParents() ) {
        if (!theFeature->tagSize()) return blankPainters();
    }

    PossiblePainters.clear();
    QList<const FeaturePainter*> DefaultPainters;
    for (int i=0; i<theFeature->layer()->getDocument()->getPaintersSize(); ++i)
    {
        const FeaturePainter* Current = dynamic_cast<const FeaturePainter*>(theFeature->layer()->getDocument()->getPainter(i));
        switch (Current->matchesTag(theFeature,NULL)) {
        case TagSelect_Match:
            PossiblePainters.push_back(Current);
            break;
        case TagSelect_DefaultMatch:
            DefaultPainters.push_back(Current);
            break;
        default:
            break;
        }
    }
    if (!PossiblePainters.size())
        PossiblePainters = DefaultPainters;
    PossiblePaintersUpToDate = true;
    HasPainter = (PossiblePainters.size() > 0);
}

void MapFeaturePrivate::updatePainters(double PixelPerM)
{
    if (!PossiblePaintersUpToDate)
        updatePossiblePainters();

    CurrentPainter = 0;
    PixelPerMForPainter = PixelPerM;
    for (int i=0; i<PossiblePainters.size(); ++i)
        if (PossiblePainters[i]->matchesZoom(PixelPerM))
        {
            CurrentPainter = PossiblePainters[i];
            return;
        }
}

void MapFeaturePrivate::blankPainters()
{
    CurrentPainter = 0;
    PossiblePainters.clear();
    PossiblePaintersUpToDate = true;
    HasPainter = false;
}

const FeaturePainter* Feature::getPainter(double PixelPerM) const
{
    if (p->PixelPerMForPainter != PixelPerM)
        p->updatePainters(PixelPerM);
    return p->CurrentPainter;
}

const FeaturePainter* Feature::getCurrentPainter() const
{
    return p->CurrentPainter;
}

bool Feature::hasPainter() const
{
    if (!p->PossiblePaintersUpToDate)
        p->updatePossiblePainters();

    return p->HasPainter;
}

bool Feature::hasPainter(double PixelPerM) const
{
    if (p->PixelPerMForPainter != PixelPerM)
        p->updatePainters(PixelPerM);
    return (p->CurrentPainter != NULL);
}

void Feature::setParentFeature(Feature* F)
{
    if (std::find(p->Parents.begin(),p->Parents.end(),F) == p->Parents.end())
        p->Parents.push_back(F);
}

void Feature::unsetParentFeature(Feature* F)
{
    for (int i=0; i<p->Parents.size(); ++i)
        if (p->Parents[i] == F)
        {
            p->Parents.erase(p->Parents.begin()+i);
            return;
        }
}

void Feature::updateIndex()
{
    if (isDeleted())
        return;

    if (layer()) {
        layer()->indexRemove(boundingBox(false), this);
        layer()->indexAdd(boundingBox(true), this);
    }
}

void Feature::updateFilters()
{
    p->FilterLayers.clear();

    Layer* L = layer();
    if (!L)
        return;

    Document* D = L->getDocument();
    if (!D)
        return;

    for (int i=0; i<D->layerSize(); ++i) {
        if (D->getLayer(i)->classType() == Layer::FilterLayerType) {
            FilterLayer* Fl = dynamic_cast<FilterLayer*>(D->getLayer(i));
            if (!Fl->isEnabled() || !Fl->selector())
                continue;
            if (Fl->selector()->matches(this, 0) != TagSelect_NoMatch)
                p->FilterLayers << Fl;
        }
    }
    invalidateMeta();
}

void Feature::updateMeta()
{
    updateFilters();

    Layer* L = layer();
    if (!L)
        return;

    if (!L->isVisible())
        p->Visible = false;
    else {
        p->Visible = true;
        foreach(FilterLayer* Fl, p->FilterLayers) {
            if (!Fl->isVisible()) {
                p->Visible = false;
                break;
            }
        }
    }

    if (L->getAlpha() != 1.0)
        p->Alpha = L->getAlpha();
    else {
        p->Alpha = 1.0;
        foreach(FilterLayer* Fl, p->FilterLayers) {
            if (Fl->getAlpha() != 1) {
                p->Alpha = Fl->getAlpha();
                break;
            }
        }
    }

    if (L->isReadonly())
        setReadonly(true);
    else {
        setReadonly(false);
        foreach(FilterLayer* Fl, p->FilterLayers) {
            if (Fl->isReadonly()) {
                setReadonly(true);
                break;
            }

        }
    }
}

int Feature::sizeParents() const
{
    return p->Parents.size();
}

IFeature* Feature::getParent(int i)
{
    return p->Parents[i];
}

const IFeature* Feature::getParent(int i) const
{
    return p->Parents[i];
}

void Feature::notifyChanges()
{
    static int Id = 0;
    notifyParents(++Id);
}

void Feature::notifyParents(int Id)
{
    if (Id != p->LastPartNotification)
    {
        p->LastPartNotification = Id;
        for (int i=0; i<p->Parents.size(); ++i)
            p->Parents[i]->partChanged(this, Id);
    }
}


void Feature::drawHover(QPainter& thePainter, MapView* theView)
{
    QPen TP(M_PREFS->getHoverColor(),M_PREFS->getHoverWidth(),Qt::SolidLine);
    thePainter.setPen(TP);

    drawSpecial(thePainter, TP, theView);

    drawChildrenSpecial(thePainter, TP, theView, 1);

    if (M_PREFS->getShowParents()) {
        TP.setDashPattern(M_PREFS->getParentDashes());
        thePainter.setPen(TP);
        drawParentsSpecial(thePainter, TP, theView);
    }
}

void Feature::drawFocus(QPainter& thePainter, MapView* theView)
{
    QPen TP(M_PREFS->getFocusColor(),M_PREFS->getFocusWidth(),Qt::SolidLine);

    thePainter.setPen(TP);

    drawSpecial(thePainter, TP, theView);

    drawChildrenSpecial(thePainter, TP, theView, 1);

    if (M_PREFS->getShowParents()) {
        TP.setDashPattern(M_PREFS->getParentDashes());
        thePainter.setPen(TP);
        drawParentsSpecial(thePainter, TP, theView);
    }
}

void Feature::drawHighlight(QPainter& thePainter, MapView* theView)
{
    QPen TP(M_PREFS->getHighlightColor(),M_PREFS->getHighlightWidth(),Qt::SolidLine);
    thePainter.setPen(TP);
    drawSpecial(thePainter, TP, theView);

    drawChildrenSpecial(thePainter, TP, theView, 1);

    if (M_PREFS->getShowParents()) {
        TP.setDashPattern(M_PREFS->getParentDashes());
        thePainter.setPen(TP);
        drawParentsSpecial(thePainter, TP, theView);
    }
}

QString Feature::toXML(int lvl, QProgressDialog * progress)
{
    QDomDocument doc;
    QDomElement root = doc.createElement("root");
    doc.appendChild(root);
    if (toXML(root, progress)) {
        QString ret;
        QTextStream ts(&ret);
        root.firstChild().save(ts, lvl);
        return ret;
//        return theXmlDoc.toString(lvl);
    } else
        return "";
}

void Feature::fromXML(const QDomElement& e, Feature* F)
{
    bool Deleted = (e.attribute("deleted") == "true");
    int Dirty = (e.hasAttribute("dirtylevel") ? e.attribute("dirtylevel").toInt() : 0);
    bool Uploaded = (e.attribute("uploaded") == "true");
    bool Special = (e.attribute("special") == "true");
    bool Selected = (e.attribute("selected") == "true");

    QDateTime time;
    time = QDateTime::fromString(e.attribute("timestamp").left(19), Qt::ISODate);
    QString user = e.attribute("user");
    int Version = e.attribute("version").toInt();
    if (Version < 1)
        Version = 0;
    Feature::ActorType A = (Feature::ActorType)(e.attribute("actor", "2").toInt());

    F->setLastUpdated(A);
    F->setDeleted(Deleted);
    F->setDirtyLevel(Dirty);
    F->setUploaded(Uploaded);
    F->setSpecial(Special);
    F->setTime(time);
    F->setUser(user);
    F->setVersionNumber(Version);

    // TODO Manage selection at document level
//    if(Selected)
//        g_Merk_MainWindow->properties()->addSelection(F);
}

void Feature::toXML(QDomElement& e, bool strict)
{
    e.setAttribute("id", xmlId());
    e.setAttribute("timestamp", time().toString(Qt::ISODate)+"Z");
    e.setAttribute("version", versionNumber());
    e.setAttribute("user", user());
    if (!strict) {
        e.setAttribute("actor", (int)lastUpdated());
        if (isDeleted())
            e.setAttribute("deleted","true");
        if (getDirtyLevel())
            e.setAttribute("dirtylevel", getDirtyLevel());
        if (isUploaded())
            e.setAttribute("uploaded","true");
        if (isSpecial())
            e.setAttribute("special","true");
        // TODO Manage selection at document level
        if (g_Merk_MainWindow->properties()->isSelected(this))
            e.setAttribute("selected","true");
    }
}

bool Feature::tagsToXML(QDomElement xParent, bool strict)
{
    bool OK = true;

    for (int i=0; i<tagSize(); ++i)
    {
        if (strict) {
            if (tagKey(i).startsWith('_') && (tagKey(i).endsWith('_')))
                continue;
        }

        QDomElement e = xParent.ownerDocument().createElement("tag");
        xParent.appendChild(e);

        e.setAttribute("k", tagKey(i));
        e.setAttribute("v", tagValue(i));
    }

    return OK;
}

void Feature::tagsFromXML(Document* d, Feature * f, QDomElement e)
{
    Q_UNUSED(d)
    QDomElement c = e.firstChildElement();
    while(!c.isNull()) {
        if (c.tagName() == "tag") {
            f->setTag(c.attribute("k"),c.attribute("v"));
        }
        c = c.nextSiblingElement();
    }
}

Relation * Feature::GetSingleParentRelation(Feature * mapFeature)
{
    int parents = mapFeature->sizeParents();

    if (parents == 0)
        return NULL;

    Relation * parentRelation = NULL;

    int i;
    for (i=0; i<parents; i++)
    {
        Relation * rel = dynamic_cast<Relation*>(mapFeature->getParent(i));
        if (!rel || rel->isDeleted()) continue;

        if (parentRelation)
            return NULL;

        if (rel->layer()->isEnabled())
            parentRelation = rel;
    }

    return parentRelation;
}

//Static
Node* Feature::getTrackPointOrCreatePlaceHolder(Document *theDocument, Layer *theLayer, const IFeature::FId& Id)
{
    Node* Part = CAST_NODE(theDocument->getFeature(Id));
    if (!Part)
    {
        Part = new Node(Coord(0,0));
        Part->setId(Id);
        Part->setLastUpdated(Feature::NotYetDownloaded);
        if (!theLayer)
            theLayer = theDocument->getDirtyOrOriginLayer();
        theLayer->add(Part);
    }
    return Part;
}

Way* Feature::getWayOrCreatePlaceHolder(Document *theDocument, Layer *theLayer, const IFeature::FId& Id)
{
    Way* Part = CAST_WAY(theDocument->getFeature(Id));
    if (!Part)
    {
        Part = new Way;
        Part->setId(Id);
        Part->setLastUpdated(Feature::NotYetDownloaded);
        if (!theLayer)
            theLayer = theDocument->getDirtyOrOriginLayer();
        theLayer->add(Part);

    }
    return Part;
}

Relation* Feature::getRelationOrCreatePlaceHolder(Document *theDocument, Layer *theLayer, const IFeature::FId& Id)
{
    Relation* Part = CAST_RELATION(theDocument->getFeature(Id));
    if (!Part)
    {
        Part = new Relation;
        Part->setId(Id);
        Part->setLastUpdated(Feature::NotYetDownloaded);
        if (!theLayer)
            theLayer = theDocument->getDirtyOrOriginLayer();
        theLayer->add(Part);
    }
    return Part;
}

void Feature::mergeTags(Document* theDocument, CommandList* L, Feature* Dest, Feature* Src)
{
    for (int i=0; i<Src->tagSize(); ++i)
    {
        QString k = Src->tagKey(i);
        QString v1 = Src->tagValue(i);
        int j = Dest->findKey(k);
        if (j == -1)
            L->add(new SetTagCommand(Dest,k,v1, theDocument->getDirtyOrOriginLayer(Dest->layer())));
        else
        {
            QString v2 = Dest->tagValue(j);
            if (v1 != v2)
            {
                L->add(new SetTagCommand(Dest,k,QString("%1;%2").arg(v2).arg(v1), theDocument->getDirtyOrOriginLayer(Dest->layer())));
            }
        }
    }
}


QString Feature::toMainHtml(QString type, QString systemtype)
{
    QString desc;
    QString name(tagValue("name",""));
    if (!name.isEmpty())
        desc = QString("<big><b>%1</b></big><br/><small>(%2)</small>").arg(name).arg(id().numId);
    else
        desc = QString("<big><b>%1</b></big>").arg(id().numId);

    QString S =
    "<html><head/><body>"
    "<small><i>" + type + "</i></small><br/>"
    + desc +
    "<br/>"
    "<i>"
    "<small>";
    S += QApplication::translate("MapFeature", "<i>V: </i><b>%1</b> ").arg(QString::number(versionNumber()));
    if (!user().isEmpty())
        S += QApplication::translate("MapFeature", "<i>last: </i><b>%1</b> by <b>%2</b>").arg(time().toString(Qt::SystemLocaleDate)).arg(user());
    else
        S += QApplication::translate("MapFeature", "<i>last: </i><b>%1</b>").arg(time().toString(Qt::SystemLocaleDate));

    if (layer())
        S += QApplication::translate("MapFeature", "<br/><i>layer: </i><b>%1</b> ").arg(layer()->name());
    S += "</small>"
    "<hr/>"
    "%1";

    if (tagSize()) {
        QStringList sTags;
        int t=0;
        for (int i=0; i<tagSize(); ++i) {
            if (tagKey(i) != "_description_" && tagKey(i) != "_comment_") {
                ++t;
                sTags << tagKey(i) + "&nbsp;=&nbsp;" + "<b>" + tagValue(i) + "</b>";
            }
        }

        if (t) {
            S += "<hr/><small>";
            S += sTags.join("<br/>");
            S += "</small>";
        }
    }

    if (hasOSMId()) {
        S += "<hr/>"
        "<a href='/" + systemtype + "/" + xmlId() + "/history'>"+QApplication::translate("MapFeature", "History")+"</a>";
        if (systemtype == "node") {
            S += "<br/>"
            "<a href='/" + systemtype + "/" + xmlId() + "/ways'>"+QApplication::translate("MapFeature", "Referenced by ways")+"</a>";
        }
        S += "<br/>"
        "<a href='/" + systemtype + "/" + xmlId() + "/relations'>"+QApplication::translate("MapFeature", "Referenced by relation")+"</a>";
    }
//	QString bbox("bbox=%1,%2,%3,%4");
//	bbox = bbox.arg(intToAng(boundingBox().bottomLeft().lon()), 0, 'f').arg(intToAng(boundingBox().bottomLeft().lat()), 0, 'f').arg(intToAng(boundingBox().topRight().lon()), 0, 'f').arg(intToAng(boundingBox().topRight().lat()), 0, 'f');
//	S += "<br/>"
//		"<a href='" + "/changesets?" + bbox + "'>"+QApplication::translate("MapFeature", "Changesets in feature bbox")+"</a>";
    S += "</body></html>";

    return S;
}

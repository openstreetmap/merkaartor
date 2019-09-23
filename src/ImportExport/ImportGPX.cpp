#include "ImportGPX.h"

#include "Command.h"
#include "DocumentCommands.h"
#include "Document.h"
#include "Node.h"
#include "TrackSegment.h"
#include "Global.h"

#include <QBuffer>
#include <QDateTime>
#include <QFile>
#include <QMessageBox>
#include <QDomDocument>
#include <QProgressDialog>


static TrackNode* importTrkPt(const QDomElement& Root, Document* /* theDocument */, Layer* theLayer)
{
    qreal Lat = Root.attribute("lat").toDouble();
    qreal Lon = Root.attribute("lon").toDouble();

    TrackNode* Pt = g_backend.allocTrackNode(theLayer, Coord(Lon,Lat));
    Pt->setLastUpdated(Feature::Log);
    if (Root.hasAttribute("xml:id"))
        Pt->setId(IFeature::FId(IFeature::Point, Root.attribute("xml:id").toLongLong()));

    theLayer->add(Pt);

    if (Root.tagName() == "wpt")
        Pt->setTag("_waypoint_", "yes");

    bool hasTimestamp = false;
    for(QDomNode n = Root.firstChild(); !n.isNull(); n = n.nextSibling())
    {
        QDomElement t = n.toElement();

        if (t.isNull())
            continue;

        if (t.tagName() == "time")
        {
            QString Value;
            for (QDomNode s = t.firstChild(); !s.isNull(); s = s.nextSibling())
            {
                QDomText ss = s.toText();
                if (!ss.isNull())
                    Value += ss.data();
            }
            if (!Value.isEmpty())
            {
                QDateTime dt(QDateTime::fromString(Value.left(19), Qt::ISODate));
                dt.setTimeSpec(Qt::UTC);
                Pt->setTime(dt);
                hasTimestamp = true;
            }
        }
        else if (t.tagName() == "ele")
        {
            Pt->setElevation( t.text().toDouble() );
        }
        else if (t.tagName() == "speed")
        {
            Pt->setSpeed( t.text().toDouble() );
        }
        else if (t.tagName() == "name")
        {
            Pt->setTag("name", t.text());
        }
        else if (t.tagName() == "desc")
        {
            Pt->setTag("_description_", t.text());
        }
        else if (t.tagName() == "cmt")
        {
            Pt->setTag("_comment_", t.text());
        }
        else if (t.tagName() == "extensions") // for OpenStreetBugs
        {
            QDomNodeList li = t.elementsByTagName("id");
            if (li.size()) {
                QString id = li.at(0).toElement().text();
                Pt->setId(IFeature::FId(IFeature::Point | IFeature::Special, id.toLongLong()));
                Pt->setTag("_special_", "yes"); // Assumed to be OpenstreetBugs as they don't use their own namesoace
                Pt->setSpecial(true);
            }
        }
    }
    if (!hasTimestamp) {
        /* If a point does not have timestamp, make sure this is reflected in
         * our representation (by default, a Feature has time of creation as a
         * timestamp).
         */
        Pt->setTime(0);
    }

    return Pt;
}


static void importTrkSeg(const QDomElement& Root, Document* theDocument, Layer* theLayer, ImportGPX::Options importOptions, QProgressDialog & progress)
{
    TrackSegment* S = g_backend.allocSegment(theLayer);
    theLayer->add(S);

    if (Root.hasAttribute("xml:id"))
        S->setId(IFeature::FId(IFeature::GpxSegment, Root.attribute("xml:id").toLongLong()));

    Node* lastPoint = NULL;

    /* Counters to keep the number of found normal and anonymized (if detection is enabled) points. */
    int nAnon = 0, nNormal = 0;

    for(QDomNode n = Root.firstChild(); !n.isNull(); n = n.nextSibling())
    {
        QDomElement t = n.toElement();
        if (t.isNull() || t.tagName() != "trkpt")
            continue;

        progress.setValue(progress.value()+1);
        if (progress.wasCanceled())
            return;

        TrackNode* Pt = importTrkPt(t,theDocument, theLayer);

        if (importOptions.testFlag( ImportGPX::Option::MakeSegmented ) && lastPoint)
        {
            qreal kilometer = Pt->position().distanceFrom( lastPoint->position() );

            if (M_PREFS->getMaxDistNodes() != 0.0 && kilometer > M_PREFS->getMaxDistNodes())
            {
                /* FIXME: This code should never trigger, as we always add a point to each created
                 * segment (and we won't execute in the first pass due to lastPoint == nullptr).
                 * Add Q_ASSERT(S.size()) instead?) */
                if (!S->size()) {
                    theLayer->remove(S);
                    g_backend.deallocFeature(theLayer, S);
                }

                S = g_backend.allocSegment(theLayer);
                theLayer->add(S);
            }
        }

        /* If the point is marked as anonymized, don't add it to a segment.
         * These are sorted by coordinates and are not proper segments. */
        if ( importOptions.testFlag(ImportGPX::Option::DetectAnonymizedSegments)
             && (Pt->time().toTime_t() == 0)
        ) {
            theLayer->add(Pt);
            nAnon++;
        } else {
            S->add(Pt);
            lastPoint = Pt;
            nNormal++;
        }
    }

    if (!S->size()) {
        theLayer->remove(S);
        g_backend.deallocFeature(theLayer, S);
    }

    /* If some anonymized points were found, mark the layer as "Anonymized
     * trackpoints". If some non-anonymized points were found as well, report a
     * warning, because that means our heuristicks failed on these points.
     *
     * Can only trigger if anonymization detection was enabled using the
     * Option::DetectAnonymizedSegments (otherwise, nAnon always stays 0,
     * regardless of the data).
     * */
    if (nAnon) {
        theLayer->setName(ImportGPX::tr("Anonymized trackpoints", "OSM trackpoints import"));
        if (nNormal) {
            qWarning() << "BUG: Mixed anonymized ( " << nAnon << ") and non-anonymized points (" << nNormal << ") found. Please report a bug.";
        }
    }
}

static void importRte(const QDomElement& Root, Document* theDocument, Layer* theLayer, ImportGPX::Options importOptions, QProgressDialog & progress)
{
    TrackSegment* S = g_backend.allocSegment(theLayer);
    theLayer->add(S);

    if (Root.hasAttribute("xml:id"))
        S->setId(IFeature::FId(IFeature::GpxSegment, Root.attribute("xml:id").toLongLong()));

    TrackNode* lastPoint = NULL;

    for(QDomNode n = Root.firstChild(); !n.isNull(); n = n.nextSibling())
    {
        QDomElement t = n.toElement();
        if (!t.isNull() && t.tagName() == "name") {
            theLayer->setName(t.text());
        } else
        if (!t.isNull() && t.tagName() == "desc") {
            theLayer->setDescription(t.text());
        } else
        if (!t.isNull() && t.tagName() == "rtept") {

            progress.setValue(progress.value()+1);
            if (progress.wasCanceled())
                return;

            TrackNode* Pt = importTrkPt(t,theDocument, theLayer);

            if (! importOptions.testFlag( ImportGPX::Option::MakeSegmented ))
                continue;

            if (lastPoint)
            {
                qreal kilometer = Pt->position().distanceFrom( lastPoint->position() );

                if (M_PREFS->getMaxDistNodes() != 0.0 && kilometer > M_PREFS->getMaxDistNodes())
                {
                    if (!S->size())
                        g_backend.deallocFeature(theLayer, S);

                    S = g_backend.allocSegment(theLayer);
                }
            }
            S->add(Pt);
            lastPoint = Pt;
        }
    }

    if (!S->size())
        g_backend.deallocFeature(theLayer, S);
}

static void importTrk(const QDomElement& Root, Document* theDocument, Layer* theLayer, ImportGPX::Options importOptions, QProgressDialog & progress)
{
    QDomElement trkSegElement;
    for(QDomNode n = Root.firstChild(); !n.isNull(); n = n.nextSibling())
    {
        QDomElement t = n.toElement();
        if (!t.isNull() && t.tagName() == "trkseg") {
            trkSegElement = t;
        } else
        if (!t.isNull() && t.tagName() == "name") {
            theLayer->setName(t.text());
        } else
        if (!t.isNull() && t.tagName() == "desc") {
            theLayer->setDescription(t.text());
        }
    }

    if (!trkSegElement.isNull()) {
        importTrkSeg(trkSegElement, theDocument, theLayer, importOptions, progress);
        if (progress.wasCanceled())
            return;
    }
}

static void importGPX(const QDomElement& Root, Document* theDocument, QList<TrackLayer*>& theTracklayers, ImportGPX::Options importOptions, QProgressDialog & progress)
{
    for(QDomNode n = Root.firstChild(); !n.isNull(); n = n.nextSibling())
    {
        QDomElement t = n.toElement();
        if (t.isNull())
            continue;

        if (t.tagName() == "trk")
        {
            TrackLayer* newLayer = new TrackLayer();
            theDocument->add(newLayer);
            importTrk(t,theDocument, newLayer, importOptions, progress);
            if (!newLayer->size()) {
                theDocument->remove(newLayer);
                delete newLayer;
            } else {
                theTracklayers.append(newLayer);
            }
        }
        else if (t.tagName() == "rte")
        {
            TrackLayer* newLayer = new TrackLayer();
            theDocument->add(newLayer);
            importRte(t,theDocument, newLayer, importOptions, progress);
            if (!newLayer->size()) {
                theDocument->remove(newLayer);
                delete newLayer;
            } else {
                theTracklayers.append(newLayer);
            }
        }
        else if (t.tagName() == "wpt")
        {
            importTrkPt(t,theDocument, theTracklayers[0]);
            progress.setValue(progress.value()+1);
        }
        if (progress.wasCanceled())
            return;
    }
}

static bool importGPX(QWidget* aParent, QIODevice& File, Document* theDocument, QList<TrackLayer*>& theTracklayers, ImportGPX::Options importOptions)
{
    // TODO remove debug messageboxes
    QDomDocument DomDoc;
    QString ErrorStr;
    int ErrorLine;
    int ErrorColumn;
    if (!DomDoc.setContent(&File, true, &ErrorStr, &ErrorLine,&ErrorColumn))
    {
        File.close();
        QMessageBox::warning(aParent,"Parse error",
            QString("Parse error at line %1, column %2:\n%3")
                                  .arg(ErrorLine)
                                  .arg(ErrorColumn)
                                  .arg(ErrorStr));
        return false;
    }
    QDomElement root = DomDoc.documentElement();
    if (root.tagName() != "gpx")
    {
        QMessageBox::information(aParent, "Parse error","Root is not a gpx node");
        return false;
    }

    QProgressDialog progress("Importing GPX...", "Cancel", 0, 0);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMaximum(progress.maximum() + DomDoc.elementsByTagName("trkpt").count());
    progress.setMaximum(progress.maximum() + DomDoc.elementsByTagName("wpt").count());

    importGPX(root, theDocument, theTracklayers, importOptions, progress);

    progress.setValue(progress.maximum());
    if (progress.wasCanceled())
        return false;

    return true;
}


bool ImportGPX::import(QWidget* aParent, const QString& aFilename, Document* theDocument, QList<TrackLayer*>& theTracklayers)
{
    QFile File(aFilename);
    if (!File.open(QIODevice::ReadOnly))
    {
        return false;
    }
    return importGPX(aParent, File, theDocument, theTracklayers, Option::MakeSegmented);
}

bool ImportGPX::import(QWidget* aParent, QByteArray& aFile, Document* theDocument, QList<TrackLayer*>& theTracklayers, ImportGPX::Options importOptions)
{
    QBuffer buf(&aFile);
    return importGPX(aParent,buf, theDocument, theTracklayers, importOptions);
}

#include "ImportGPX.h"

#include "Command.h"
#include "DocumentCommands.h"
#include "Document.h"
#include "Node.h"
#include "TrackSegment.h"

#include <QtCore/QBuffer>
#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtGui/QMessageBox>
#include <QtXml/QDomDocument>
#include <QProgressDialog>


static Node* importTrkPt(const QDomElement& Root, Document* /* theDocument */, Layer* theLayer)
{
    double Lat = Root.attribute("lat").toDouble();
    double Lon = Root.attribute("lon").toDouble();

    Node* Pt = new Node(Coord(angToCoord(Lat),angToCoord(Lon)));
    Pt->setLastUpdated(Feature::Log);
    if (Root.hasAttribute("xml:id"))
        Pt->setId(Root.attribute("xml:id"));

    theLayer->add(Pt);

    if (Root.tagName() == "wpt")
        Pt->setTag("_waypoint_", "yes");

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
                Pt->setId("osb_" + id);
                Pt->setTag("_special_", "yes"); // Assumed to be OpenstreetBugs as they don't use their own namesoace
                Pt->setSpecial(true);
            }
        }
    }

    return Pt;
}


static void importTrkSeg(const QDomElement& Root, Document* theDocument, Layer* theLayer, bool MakeSegment, QProgressDialog & progress)
{
    TrackSegment* S = new TrackSegment;

    if (Root.hasAttribute("xml:id"))
        S->setId(Root.attribute("xml:id"));

    Node* lastPoint = NULL;

    for(QDomNode n = Root.firstChild(); !n.isNull(); n = n.nextSibling())
    {
        QDomElement t = n.toElement();
        if (t.isNull() || t.tagName() != "trkpt")
            continue;

        progress.setValue(progress.value()+1);
        if (progress.wasCanceled())
            return;

        Node* Pt = importTrkPt(t,theDocument, theLayer);

        if (MakeSegment == false)
            continue;

        if (lastPoint)
        {
            double kilometer = Pt->position().distanceFrom( lastPoint->position() );

            if (M_PREFS->getMaxDistNodes() != 0.0 && kilometer > M_PREFS->getMaxDistNodes())
            {
                if (S->size())
                    theLayer->add(S);
                else
                    delete S;

                S = new TrackSegment;
            }
        }

        S->add(Pt);
        lastPoint = Pt;
    }

    if (S->size())
        theLayer->add(S);
    else
        delete S;
}

static void importRte(const QDomElement& Root, Document* theDocument, Layer* theLayer, bool MakeSegment, QProgressDialog & progress)
{
    TrackSegment* S = new TrackSegment;

    if (Root.hasAttribute("xml:id"))
        S->setId(Root.attribute("xml:id"));

    Node* lastPoint = NULL;

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

            Node* Pt = importTrkPt(t,theDocument, theLayer);

            if (MakeSegment == false)
                continue;

            if (lastPoint)
            {
                double kilometer = Pt->position().distanceFrom( lastPoint->position() );

                if (M_PREFS->getMaxDistNodes() != 0.0 && kilometer > M_PREFS->getMaxDistNodes())
                {
                    if (S->size())
                        theLayer->add(S);
                    else
                        delete S;

                    S = new TrackSegment;
                }
            }
            S->add(Pt);
            lastPoint = Pt;
        }
    }

    if (S->size())
        theLayer->add(S);
    else
        delete S;
}

static void importTrk(const QDomElement& Root, Document* theDocument, Layer* theLayer, bool MakeSegment, QProgressDialog & progress)
{
    for(QDomNode n = Root.firstChild(); !n.isNull(); n = n.nextSibling())
    {
        QDomElement t = n.toElement();
        if (!t.isNull() && t.tagName() == "trkseg") {
            importTrkSeg(t,theDocument, theLayer, MakeSegment, progress);
            if (progress.wasCanceled())
                return;
        } else
        if (!t.isNull() && t.tagName() == "name") {
            theLayer->setName(t.text());
        } else
        if (!t.isNull() && t.tagName() == "desc") {
            theLayer->setDescription(t.text());
        }
    }
}

static void importGPX(const QDomElement& Root, Document* theDocument, QList<TrackLayer*>& theTracklayers, bool MakeSegment, QProgressDialog & progress)
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
            newLayer->blockIndexing(true);
            importTrk(t,theDocument, newLayer, MakeSegment, progress);
            if (!newLayer->size()) {
                theDocument->remove(newLayer);
                delete newLayer;
            } else {
                newLayer->blockIndexing(false);
                newLayer->reIndex();
                theTracklayers.append(newLayer);
            }
        }
        else if (t.tagName() == "rte")
        {
            TrackLayer* newLayer = new TrackLayer();
            theDocument->add(newLayer);
            newLayer->blockIndexing(true);
            importRte(t,theDocument, newLayer, MakeSegment, progress);
            if (!newLayer->size()) {
                theDocument->remove(newLayer);
                delete newLayer;
            } else {
                newLayer->blockIndexing(false);
                newLayer->reIndex();
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

bool importGPX(QWidget* aParent, QIODevice& File, Document* theDocument, QList<TrackLayer*>& theTracklayers, bool MakeSegment)
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

    importGPX(root, theDocument, theTracklayers, MakeSegment, progress);

    progress.setValue(progress.maximum());
    if (progress.wasCanceled())
        return false;

    return true;
}


bool importGPX(QWidget* aParent, const QString& aFilename, Document* theDocument, QList<TrackLayer*>& theTracklayers)
{
    QFile File(aFilename);
    if (!File.open(QIODevice::ReadOnly))
    {
        return false;
    }
    return importGPX(aParent, File, theDocument, theTracklayers, true);
}

bool importGPX(QWidget* aParent, QByteArray& aFile, Document* theDocument, QList<TrackLayer*>& theTracklayers, bool MakeSegment)
{
    QBuffer buf(&aFile);
    return importGPX(aParent,buf, theDocument, theTracklayers, MakeSegment);
}

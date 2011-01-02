#include "ImportNGT.h"
#include "DocumentCommands.h"
#include "Document.h"
#include "Node.h"
#include "TrackSegment.h"
#include "Global.h"

#include <QtCore/QFile>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>

#include <math.h>

bool importNGT(QWidget* /* aParent */, const QString& aFilename, Document* theDocument, Layer* theLayer)
{
    QFile f(aFilename);
    if (f.open(QIODevice::ReadOnly))
    {
        QTextStream s(&f);
        CommandList* theList  = new CommandList(MainWindow::tr("Import NGT"), NULL);
        TrackSegment* theSegment = g_backend.allocSegment();
        while (!f.atEnd())
        {
            QString Line(f.readLine());
            QStringList Items(Line.split('|'));
            if (Items.count() >= 5)
            {
                Node* Pt = g_backend.allocNode(Coord(Items[3].toDouble()*COORD_MAX, Items[4].toDouble()*COORD_MAX));
                Pt->setLastUpdated(Feature::Log);
                theList->add(new AddFeatureCommand(theLayer,Pt, true));
                theSegment->add(Pt);
            }
        }
        if (theList->empty())
        {
            delete theList;
            g_backend.deallocFeature(theSegment);
        }
        else
        {
            theList->add(new AddFeatureCommand(theLayer,theSegment,true));
            theDocument->addHistory(theList);
        }
        return true;
    }
    return false;
}

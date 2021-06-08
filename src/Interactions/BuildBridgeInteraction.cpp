#include "Global.h"
#include "BuildBridgeInteraction.h"

#include "MainWindow.h"
#include "PropertiesDock.h"
#include "DocumentCommands.h"
#include "FeatureCommands.h"
#include "WayCommands.h"
#include "Projection.h"
#include "Node.h"
#include "LineF.h"
#include "FeatureManipulations.h"
#include "Global.h"

#include <QList>
#include <QMessageBox>
#include <assert.h>

BuildBridgeInteraction::BuildBridgeInteraction(MainWindow* aMain)
    : FeatureSnapInteraction(aMain), freshSplit(false)
{
	g_Merk_MainWindow->properties()->setSelection(0);
	g_Merk_MainWindow->view()->invalidate(true, true, false);
}

BuildBridgeInteraction::~BuildBridgeInteraction(void)
{
}

QString BuildBridgeInteraction::toHtml()
{
    QString help;
    //help = (MainWindow::tr("LEFT-CLICK to select; LEFT-DRAG to move"));

    QString desc;
    desc = QString("<big><b>%1</b></big><br/>").arg(MainWindow::tr("Build Bridge interaction"));
    desc += QString("<b>%1</b><br/>").arg(help);

    QString S =
    "<html><head/><body>"
    "<small><i>" + QString(metaObject()->className()) + "</i></small><br/>"
    + desc;
    S += "</body></html>";

    return S;
}

/*
void BuildBridgeInteraction::snapMousePressEvent(QMouseEvent * ev, Feature* aFeat)
{
	Q_UNUSED(ev);
	Q_UNUSED(aFeat);
#ifdef _MOBILE
        theMain->view()->setCursor(cursor());
#endif
}

void BuildBridgeInteraction::snapMouseMoveEvent(QMouseEvent* ev, Feature* aFeat)
{
#ifdef _MOBILE
        theMain->view()->setCursor(cursor());
#endif
}
*/

void BuildBridgeInteraction::snapMouseReleaseEvent(QMouseEvent * ev, Feature* aFeat)
{
/*
    if (theMoveInteraction) {
		QMessageBox::critical(g_Merk_MainWindow, tr("BridgeBuilder"), tr("Hmm"));
        theMoveInteraction->snapMouseReleaseEvent(ev, aFeat);
        return;
    } */


	Node* addNode = NULL;

    Way* aRoad = dynamic_cast<Way*>(aFeat);
    if (aRoad) {
        Coord P(XY_TO_COORD(ev->pos()));
        addNode = createNode(P, aFeat);
    } else if (aFeat) {
		addNode = dynamic_cast<Node*>(aFeat);
	}


	if (freshSplit) {
		freshSplit = false;
		g_Merk_MainWindow->properties()->setSelection(0);
	}

	if (addNode) {
		g_Merk_MainWindow->properties()->addSelection(addNode);
	}
	if (g_Merk_MainWindow->properties()->selectionSize() >= 2) {
		splitAndMark();
		freshSplit = true;
	}
}

void BuildBridgeInteraction::splitAndMark() {
	/* Do some sanity check */
	Node*  firstNode = CAST_NODE(g_Merk_MainWindow->properties()->selection(0));
	Node* secondNode = CAST_NODE(g_Merk_MainWindow->properties()->selection(1));
	if ((g_Merk_MainWindow->properties()->selectionSize() > 2) || (!firstNode) || (!secondNode)) {
		QMessageBox::critical(g_Merk_MainWindow, tr("BridgeBuilder"), tr("Sorry, I don't know how to build bridge from THAT. Please, give me two nodes only."));
		g_Merk_MainWindow->properties()->setSelection(0);
		return;
	}
	
    CommandList* theList = new CommandList(tr("Convert segment to bridge"), NULL);
	Way *R = cutoutRoad(g_Merk_MainWindow->document(), theList, g_Merk_MainWindow->properties(), firstNode, secondNode);
	if (R) {
		/* Add the bridge=yes/tunnel=culvert tag, including possible layer increase if nonexistent */
		if (R->findKey("bridge") == -1 && R->findKey("tunnel") == -1) {
			int layer = R->tagValue("layer", "0").toInt();
			if (R->findKey("highway") != -1) {
				theList->add(new SetTagCommand(R, "bridge", "yes"));
				layer++;
			}
			if (R->findKey("waterway") != -1) {
				theList->add(new SetTagCommand(R, "tunnel", "culvert"));
				layer--;
			}

			if (layer == 0) {
				/* Remove the layer tag if it exists and would become 0. */
				if (R->findKey("level") != -1) {
					theList->add(new ClearTagCommand(R, "layer"));
				}
			} else {
				theList->add(new SetTagCommand(R, "layer", QString::number(layer)));
			}
		} else {
			QMessageBox::critical(g_Merk_MainWindow, tr("BridgeBuilder"), tr("Selected segment is already tagged as bridge/tunnel. Please, make sure you know what you're doing."));
		}
		g_Merk_MainWindow->properties()->setSelection(R);
		g_Merk_MainWindow->document()->addHistory(theList);
		//g_Merk_MainWindow->view()->invalidate(true, true, false);
		g_Merk_MainWindow->invalidateView();
	} else {
		QMessageBox::critical(g_Merk_MainWindow, tr("BridgeBuilder"), tr("Unsupported action: The bridge does not seem to be a single way."));
		theList->undo();
	}
}

#ifndef _MOBILE
QCursor BuildBridgeInteraction::cursor() const
{
    return QCursor(Qt::CrossCursor);
}
#endif

Node *BuildBridgeInteraction::createNode(Coord P, Feature* aFeat)
{
    Node* N = NULL;
    CommandList* theList;
    Way* aRoad = dynamic_cast<Way*>(aFeat);
	/* Bridge can be only created from existing roads */
    if (aRoad)
    {
        theList  = new CommandList(MainWindow::tr("Create node in way %1").arg(aRoad->id().numId), aRoad);
        int SnapIdx = findSnapPointIndex(aRoad, P);
        N = g_backend.allocNode(g_Merk_MainWindow->document()->getDirtyOrOriginLayer(aRoad->layer()), P);
        theList->add(new AddFeatureCommand(g_Merk_MainWindow->document()->getDirtyOrOriginLayer(aRoad->layer()),N,true));
        theList->add(new WayAddNodeCommand(aRoad,N,SnapIdx,g_Merk_MainWindow->document()->getDirtyOrOriginLayer(aRoad->layer())));

		g_Merk_MainWindow->document()->addHistory(theList);
		g_Merk_MainWindow->view()->invalidate(true, true, false);
    } else {
		assert(0);
	}
	return N;
}

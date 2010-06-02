#include "Maps/FeatureManipulations.h"
#include "Coord.h"
#include "DocumentCommands.h"
#include "FeatureCommands.h"
#include "WayCommands.h"
#include "RelationCommands.h"
#include "NodeCommands.h"
#include "Document.h"
#include "Features.h"
#include "PropertiesDock.h"

#include <QtCore/QString>

#include <algorithm>

#ifndef _MOBILE
#include <ggl/ggl.hpp>
#include <ggl/geometries/cartesian2d.hpp>
#include <ggl/algorithms/intersection.hpp>
#endif

static void mergeNodes(Document* theDocument, CommandList* theList, Node *node1, Node *node2);

static bool isNear(Node* a, Node* b)
{
    // For now, only if exactly same position
    // Future: use distance threshold?
    return a->position() == b->position();
}

bool canJoin(Way* R1, Way* R2)
{
    if ( (R1->size() == 0) || (R2->size() == 0) )
        return true;
    Node* Start1 = R1->getNode(0);
    Node* End1 = R1->getNode(R1->size()-1);
    Node* Start2 = R2->getNode(0);
    Node* End2 = R2->getNode(R2->size()-1);
    return (Start1 == Start2) ||
        (Start1 == End2) ||
        (End1 == Start2) ||
        (End1 == End2) ||
        isNear(Start1, Start2) ||
        isNear(Start1, End2) ||
        isNear(End1, Start2) ||
        isNear(End1, End2);
}

bool canBreak(Way* R1, Way* R2)
{
    if ( (R1->size() == 0) || (R2->size() == 0) )
        return false;
    for (int i=0; i<R1->size(); i++)
        for (int j=0; j<R2->size(); j++)
            if (R1->get(i) == R2->get(j))
                return true;
    return false;
}

bool canJoinRoads(PropertiesDock* theDock)
{
    QHash<Coord,int> ends;
    for (int i=0; i<theDock->size(); ++i)
        if (Way* R = CAST_WAY(theDock->selection(i))) {
            if (R->isClosed()) continue;
            Coord start = R->getNode(0)->position();
            Coord end = R->getNode(R->size()-1)->position();
            if (++ends[start] > 2) return false;
            if (++ends[end] > 2) return false;
        }

    return ends.values().contains(2);
}

bool canBreakRoads(PropertiesDock* theDock)
{
    QList<Way*> Input;
    for (int i=0; i<theDock->size(); ++i)
        if (Way* R = CAST_WAY(theDock->selection(i)))
            Input.push_back(R);
    for (int i=0; i<Input.size(); ++i)
        for (int j=i+1; j<Input.size(); ++j)
            if (canBreak(Input[i],Input[j]))
                return true;
    return false;
}

bool canDetachNodes(PropertiesDock* theDock)
{
    QList<Way*> Roads, Result;
    QList<Node*> Points;
    for (int i=0; i<theDock->size(); ++i)
        if (Way* R = CAST_WAY(theDock->selection(i)))
            Roads.push_back(R);
        else if (Node* Pt = CAST_NODE(theDock->selection(i)))
            Points.push_back(Pt);

    if (Roads.size() > 1 && Points.size() > 1)
        return false;

    if (Roads.size() == 0 && Points.size()) {
        for (int i=0; i<Points.size(); ++i) {
            Way * R = Way::GetSingleParentRoad(Points[i]);
            if (R)
                return true;
        }
        return false;
    }

    if (Roads.size() && Points.size() == 1)
        return true;

    if (Roads.size() == 1 && Points.size())
        return true;

    return false;
}

void reversePoints(Document* theDocument, CommandList* theList, Way* R)
{
    Layer *layer=theDocument->getDirtyOrOriginLayer(R->layer());
    for (int i=R->size()-2; i>=0; --i)
    {
        Node* Pt = R->getNode(i);
        theList->add(new WayRemoveNodeCommand(R,i,layer));
        theList->add(new WayAddNodeCommand(R,Pt,layer));
    }
}


static double distanceFrom(QLineF ab, const Coord& c)
{
    // distance in metres from c to line segment ab
    qreal ab_len2 = ab.dx() * ab.dx() + ab.dy() * ab.dy();
    QPointF dp;
    if (ab_len2) {
        QLineF ac(ab.p1(), c.toPointF());
        float u = (ac.dx() * ab.dx() + ac.dy() * ab.dy()) / ab_len2;
        if (u < 0.0) u = 0.0;
        else if (u > 1.0) u = 1.0;
        dp = ab.pointAt(u);
    } else {
        dp = ab.p1();
    }
    Coord dc(dp);
    return dc.distanceFrom(c);
}
void simplifyWay(Document *doc, Layer *layer, CommandList *theList, Way *w, int start, int end, double threshold)
{
    // Douglas-Peucker reduction of uninteresting points in way, subject to maximum error in metres

    // TODO Performance: Use path-hull algorithm described at http://www.cs.ubc.ca/cgi-bin/tr/1992/TR-92-07.ps
    if (end - start <= 1)
        // no removable nodes
        return;

    QLineF segment(w->getNode(start)->position().toPointF(), w->getNode(end)->position().toPointF());
    qreal maxdist = -1;
    int maxpos = 0;
    for (int i = start+1;  i < end;  i++) {
        qreal d = distanceFrom(segment, w->getNode(i)->position());
        if (d > maxdist) {
            maxdist = d;
            maxpos = i;
        }
    }
    // maxdist is in kilometres
    if (maxpos && maxdist * 1000 > threshold) {
        simplifyWay(doc, layer, theList, w, maxpos, end, threshold);
        simplifyWay(doc, layer, theList, w, start, maxpos, threshold);
    } else {
        for (int i = end - 1;  i > start;  i--) {
            Feature *n = w->get(i);
            theList->add(new WayRemoveNodeCommand(w, i, layer));
            theList->add(new RemoveFeatureCommand(doc, n));
        }
    }
}
QSet<QString> uninterestingKeys;
bool isNodeInteresting(Node *n)
{
    for (int i = 0; i < n->tagSize();  i++)
        if (!uninterestingKeys.contains(n->tagKey(i)))
            return true;
    return false;
}

void simplifyRoads(Document* theDocument, CommandList* theList, PropertiesDock* theDock, double threshold)
{
    if (uninterestingKeys.isEmpty())
        uninterestingKeys << "created_by" << "source";

    for (int i = 0;  i < theDock->size();  ++i)
        if (Way* w = CAST_WAY(theDock->selection(i))) {
            Layer *layer = theDocument->getDirtyOrOriginLayer(w->layer());
            int end = w->size() - 1;
            for (int i = end;  i >= 0;  i--) {
                Node *n = w->getNode(i);
                if (i == 0 || n->sizeParents() > 1 || isNodeInteresting(n)) {
                    simplifyWay(theDocument, layer, theList, w, i, end, threshold);
                    end = i;
                }
            }
        }
}

static void appendPoints(Document* theDocument, CommandList* L, Way* Dest, Way* Src, bool prepend, bool reverse)
{
    Layer *layer = theDocument->getDirtyOrOriginLayer(Src->layer());
    int srclen = Src->size();
    int destpos = prepend ? 0 : Dest->size();
    for (int i=1; i<srclen; ++i) {
        int j = (reverse ? srclen-i : i) - (prepend != reverse ? 1 : 0);
        Node* Pt = Src->getNode(j);
        L->add(new WayAddNodeCommand(Dest, Pt, destpos++, layer));
    }
}

static Way* join(Document* theDocument, CommandList* L, Way* R1, Way* R2)
{
    QList<Feature*> Alternatives;
    if (R1->size() == 0)
    {
        Feature::mergeTags(theDocument,L,R2,R1);
        L->add(new RemoveFeatureCommand(theDocument,R1,Alternatives));
        return R2;
    }
    if (R2->size() == 0)
    {
        Feature::mergeTags(theDocument,L,R1,R2);
        L->add(new RemoveFeatureCommand(theDocument,R2,Alternatives));
        return R1;
    }
    Node* Start1 = R1->getNode(0);
    Node* End1 = R1->getNode(R1->size()-1);
    Node* Start2 = R2->getNode(0);
    Node* End2 = R2->getNode(R2->size()-1);

    bool prepend = false;       // set true if R2 meets beginning of R1
    bool reverse = false;       // set true if R2 is opposite direction to R1

    if (End1 == Start2) {
        // nothing to do, but skip the other tests
    } else if (End1 == End2) {
        reverse = true;
    } else if (Start1 == Start2) {
        prepend = true;
        reverse = true;
    } else if (Start1 == End2) {
        prepend = true;
    } else if (isNear(End1, Start2)) {
        mergeNodes(theDocument, L, End1, Start2);
    } else if (isNear(End1, End2)) {
        mergeNodes(theDocument, L, End1, End2);
        reverse = true;
    } else if (isNear(Start1, Start2)) {
        mergeNodes(theDocument, L, Start1, Start2);
        prepend = true;
        reverse = true;
    } else if (isNear(Start1, End2)) {
        mergeNodes(theDocument, L, Start1, End2);
        prepend = true;
    }

    appendPoints(theDocument, L, R1, R2, prepend, reverse);
    Feature::mergeTags(theDocument,L,R1,R2);
    L->add(new RemoveFeatureCommand(theDocument,R2,Alternatives));

    // Auto-merge nearly-closed ways
    Node *StartResult = R1->getNode(0);
    Node *EndResult = R1->getNode(R1->size()-1);
    if (StartResult != EndResult && isNear(StartResult, EndResult))
        mergeNodes(theDocument, L, StartResult, EndResult);
    return R1;
}

void joinRoads(Document* theDocument, CommandList* theList, PropertiesDock* theDock)
{
    QList<Way*> Input;
    QList<Way*> Output;
    for (int i=0; i<theDock->size(); ++i)
        if (Way* R = CAST_WAY(theDock->selection(i)))
            Input.append(R);
    while (!Input.isEmpty())
    {
        Way *R1 = Input.takeFirst();
        if (R1->size() >= 4 && !R1->isClosed() && R1->getNode(0)->position() == R1->getNode(R1->size()-1)->position()) {
            // almost-area; close it properly
            mergeNodes(theDocument, theList, R1->getNode(0), R1->getNode(R1->size()-1));
        } else {
            for (int i = 0;  i < Input.size();  ++i) {
                Way *R2 = Input[i];
                if (canJoin(R1, R2)) {
                    R1 = join(theDocument, theList, R1, R2);
                    Input.removeAt(i);
                    i = -1;        // restart the loop
                }
            }
        }
        Output.append(R1);
    }
    theDock->setSelection(Output);
}

static bool handleWaysplitSpecialRestriction(Document* theDocument, CommandList* theList, Way* FirstPart, Way* NextPart, Relation* theRel)
{
    if (theRel->tagValue("type","") != "restriction")
        return false;

    for (int k=0; k < theRel->size(); k++)
    {
        // check for via
        if (theRel->getRole(k) == "via")
        {
            if (theRel->get(k) == FirstPart)
            {
                // whoops, we just split a via way, just add the new way, too
                theList->add(new RelationAddFeatureCommand(theRel, theRel->getRole(k), NextPart, k+1, theDocument->getDirtyOrOriginLayer(FirstPart->layer())));
                // we just added a member, so get over it
                k++;
            }
            else if ((theRel->get(k))->getType() == Feature::Nodes)
            {
                // this seems to be a via node, check if it is member the nextPart
                if (NextPart->find(theRel->get(k)) < NextPart->size())
                {
                    // yes it is, so remove First Part and add Next Part to it
                    int idx = theRel->find(FirstPart);
                    theList->add(new RelationAddFeatureCommand(theRel, theRel->getRole(idx), NextPart, idx+1, theDocument->getDirtyOrOriginLayer(FirstPart->layer())));
                    theList->add(new RelationRemoveFeatureCommand(theRel, idx, theDocument->getDirtyOrOriginLayer(FirstPart->layer())));
                }
            }
            else if ((theRel->get(k))->getType() == Feature::Ways)
            {
                // this is a way, check the nodes
                Way* W = CAST_WAY(theRel->get(k));
                for (int l=0; l<W->size(); l++)
                {
		    // check if this node is member the nextPart
                    if (NextPart->find(W->get(l)) < NextPart->size())
                    {
                        // yes it is, so remove First Part and add Next Part to it
                        int idx = theRel->find(FirstPart);
                        theList->add(new RelationAddFeatureCommand(theRel, theRel->getRole(idx), NextPart, idx+1, theDocument->getDirtyOrOriginLayer(FirstPart->layer())));
                        theList->add(new RelationRemoveFeatureCommand(theRel, idx, theDocument->getDirtyOrOriginLayer(FirstPart->layer())));
                        break;
                    }
                }
            }
        }
    }
    return true;
}

static void handleWaysplitRelations(Document* theDocument, CommandList* theList, Way* FirstPart, Way* NextPart)
{
    /* since we may delete First Part from some Relations here, we first build a list of the relations to check */
    QList<Relation*> checkList;

    for (int j=0; j < FirstPart->sizeParents(); j++) {
        checkList.append(CAST_RELATION(FirstPart->getParent(j)));
    }

    for (int j=0; j < checkList.count(); j++) {
        Relation* L = checkList.at(j);
        if (!handleWaysplitSpecialRestriction(theDocument, theList, FirstPart, NextPart, L))
        {
            int idx = L->find(FirstPart);
            theList->add(new RelationAddFeatureCommand(L, L->getRole(idx), NextPart, idx+1, theDocument->getDirtyOrOriginLayer(FirstPart->layer())));
        }
    }
}


static void splitRoad(Document* theDocument, CommandList* theList, Way* In, const QList<Node*>& Points, QList<Way*>& Result)
{
    int pos;
    if (In->isClosed()) {  // Special case: If area, rotate the area so that the start node is the first point of splitting

        QList<Node*> Target;
        for (int i=0; i < Points.size(); i++)
            if ((pos = In->find(Points[i])) != In->size()) {
                for (int j=pos+1; j<In->size(); ++j)
                    Target.push_back(In->getNode(j));
                for (int j=1; j<= pos; ++j)
                    Target.push_back(In->getNode(j));
                break;
            }
        if (pos == In->size())
            return;

        if (Points.size() == 1) // Special case: For a 1 point area splitting, de-close the road, i.e. duplicate the selected node
        {
            Node* N = new Node(*(In->getNode(pos)));
            theList->add(new AddFeatureCommand(theDocument->getDirtyOrOriginLayer(In->layer()),N,true));

            Target.prepend(N);
        } else // otherwise, just close the modified area
            Target.prepend(In->getNode(pos));

        // Now, reconstruct the road/area
        while (In->size())
            theList->add(new WayRemoveNodeCommand(In,(int)0,theDocument->getDirtyOrOriginLayer(In->layer())));

        for (int i=0; i<Target.size(); ++i)
            theList->add(new WayAddNodeCommand(In,Target[i],theDocument->getDirtyOrOriginLayer(In->layer())));

        if (Points.size() == 1) {  // For 1-point, we are done
            Result.push_back(In);
            return;
        }
    }

    Way* FirstPart = In;
    Result.push_back(FirstPart);
    for (int i=1; (i+1)<FirstPart->size(); ++i)
    {
        if (std::find(Points.begin(),Points.end(),FirstPart->get(i)) != Points.end())
        {
            Way* NextPart = new Way;
            copyTags(NextPart,FirstPart);
            theList->add(new AddFeatureCommand(theDocument->getDirtyOrOriginLayer(In->layer()),NextPart,true));
            theList->add(new WayAddNodeCommand(NextPart, FirstPart->getNode(i), theDocument->getDirtyOrOriginLayer(In->layer())));
            while ( (i+1) < FirstPart->size() )
            {
                theList->add(new WayAddNodeCommand(NextPart, FirstPart->getNode(i+1), theDocument->getDirtyOrOriginLayer(In->layer())));
                theList->add(new WayRemoveNodeCommand(FirstPart,i+1,theDocument->getDirtyOrOriginLayer(In->layer())));
            }
            handleWaysplitRelations(theDocument, theList, FirstPart, NextPart);

            Result.push_back(NextPart);
            FirstPart = NextPart;
            i=0;
        }
    }


}

void splitRoads(Document* theDocument, CommandList* theList, PropertiesDock* theDock)
{
    QList<Way*> Roads, Result;
    QList<Node*> Points;
    for (int i=0; i<theDock->size(); ++i)
        if (Way* R = CAST_WAY(theDock->selection(i)))
            Roads.push_back(R);
        else if (Node* Pt = CAST_NODE(theDock->selection(i)))
            Points.push_back(Pt);

    if (Roads.size() == 0 && Points.size() == 1)
    {
        Way * R = Way::GetSingleParentRoadInner(Points[0]);
        if (R)
            Roads.push_back(R);
    }

    for (int i=0; i<Roads.size(); ++i)
        splitRoad(theDocument, theList,Roads[i],Points, Result);
    theDock->setSelection(Result);
}

static void breakRoad(Document* theDocument, CommandList* theList, Way* R, Node* Pt)
{
    for (int i=0; i<R->size(); ++i)
        if (R->get(i) == Pt)
        {
            Node* New = new Node(*Pt);
            copyTags(New,Pt);
            theList->add(new AddFeatureCommand(theDocument->getDirtyOrOriginLayer(R->layer()),New,true));
            theList->add(new WayRemoveNodeCommand(R,i,theDocument->getDirtyOrOriginLayer(R->layer())));
            theList->add(new WayAddNodeCommand(R,New,i,theDocument->getDirtyOrOriginLayer(R->layer())));
        }
        if (!Pt->sizeParents())
            theList->add(new RemoveFeatureCommand(theDocument,Pt));
}

void breakRoads(Document* theDocument, CommandList* theList, PropertiesDock* theDock)
{
    QList<Way*> Roads, Result;
    QList<Node*> Points;
    for (int i=0; i<theDock->size(); ++i)
        if (Way* R = CAST_WAY(theDock->selection(i)))
            Roads.push_back(R);
        else if (Node* Pt = CAST_NODE(theDock->selection(i)))
            Points.push_back(Pt);

    if (Roads.size() == 0 && Points.size() == 1)
    {
        for (int i=0; i<Points[0]->sizeParents() ; ++i) {
            Way * R = CAST_WAY(Points[0]->getParent(i));
            if (R && !R->isDeleted())
                Roads.push_back(R);
        }
    }

    if (Roads.size() == 1 && Points.size() ) {
        splitRoad(theDocument, theList,Roads[0],Points, Result);
        if (Roads[0]->area() > 0.0) {
            for (int i=0; i<Points.size(); ++i)
                breakRoad(theDocument, theList, Roads[0],Points[i]);
        } else {
            Roads = Result;
        }
    }

    for (int i=0; i<Roads.size(); ++i)
        for (int j=0; j<Roads[i]->size(); ++j)
            for (int k=i+1; k<Roads.size(); ++k)
                breakRoad(theDocument, theList, Roads[k],CAST_NODE(Roads[i]->get(j)));
}

bool canCreateJunction(PropertiesDock* theDock)
{
    return createJunction(NULL, NULL, theDock, false);
}

int createJunction(Document* theDocument, CommandList* theList, PropertiesDock* theDock, bool doIt)
{
    //TODO test that the junction do not already exists!
    typedef ggl::point_2d P;

    QList<Way*> Roads, Result;
    for (int i=0; i<theDock->size(); ++i)
        if (Way* R = CAST_WAY(theDock->selection(i)))
            Roads.push_back(R);

    if (Roads.size() < 2)
        return 0;

    Way* R1 = Roads[0];
    Way* R2 = Roads[1];

    return Way::createJunction(theDocument, theList, R1, R2, doIt);
}

#define STREET_NUMBERS_LENGTH 1500.0
#define STREET_NUMBERS_ANGLE 30.0

void createStreetNumbers(Document* theDocument, CommandList* theList, Way* theRoad, bool Left)
{
    QString streetName = theRoad->tagValue("name", "");
    QLineF l, l2, nv;

    Node* N;
    Way* R = new Way;
    theList->add(new AddFeatureCommand(theDocument->getDirtyOrOriginLayer(),R,true));
    theList->add(new SetTagCommand(R, "addr:interpolation", ""));
    theList->add(new SetTagCommand(R, "addr:street", streetName));
    QPointF prevPoint;

    for (int j=0; j < theRoad->size(); j++) {
        if (j == 0) {
            l2 = QLineF(theRoad->getNode(j)->position().toPointF(), theRoad->getNode(j+1)->position().toPointF());
            l = l2;
            l.setAngle(l2.angle() + 180.);
            prevPoint = l.p2();
        } else
        if (j == theRoad->size()-1) {
            l = QLineF(theRoad->getNode(j)->position().toPointF(), theRoad->getNode(j-1)->position().toPointF());
            l2 = l;
            l2.setAngle(l.angle() + 180.);
        } else {
            l = QLineF(theRoad->getNode(j)->position().toPointF(), theRoad->getNode(j-1)->position().toPointF());
            l2 = QLineF(theRoad->getNode(j)->position().toPointF(), theRoad->getNode(j+1)->position().toPointF());
        }
        nv = l.normalVector().unitVector();

        double theAngle = (l.angle() - l2.angle());
        if (theAngle < 0.0) theAngle = 360. + theAngle;
        theAngle /= 2.;
        nv.setAngle(l2.angle() + theAngle);
        nv.setLength(STREET_NUMBERS_LENGTH/sin(angToRad(theAngle)));
        if (Left)
            nv.setAngle(nv.angle() + 180.0);

        QLineF lto(prevPoint, nv.p2());
        lto.setLength(lto.length()+STREET_NUMBERS_LENGTH);
        QPointF pto;

        bool intersectedTo = false;
        for (int k=0; k < theRoad->getNode(j)->sizeParents(); ++k) {
            Way* I = CAST_WAY(theRoad->getNode(j)->getParent(k));
            if (!I || I == theRoad || I->isDeleted())
                continue;

            for (int m=0; m < I->size()-1; ++m) {
                QLineF l3 = QLineF(I->getNode(m)->position().toPointF(), I->getNode(m+1)->position().toPointF());
                QPointF theIntersection;
                if (lto.intersect(l3, &theIntersection) == QLineF::BoundedIntersection) {
                    intersectedTo = true;
                    QLineF lt = QLineF(prevPoint, theIntersection);
                    if (lt.length() < lto.length())
                        lto = lt;
                }
            }
        }

        if (j != 0) {
            QLineF lfrom = QLineF(nv.p2(), prevPoint);
            lfrom.setLength(lfrom.length()*2.);
            QPointF pfrom;

            bool intersectedFrom = false;
            for (int k=0; k < theRoad->getNode(j-1)->sizeParents(); ++k) {
                Way* I = CAST_WAY(theRoad->getNode(j-1)->getParent(k));
                if (!I || I == theRoad || I->isDeleted())
                    continue;

                for (int m=0; m < I->size()-1; ++m) {
                    QLineF l3 = QLineF(I->getNode(m)->position().toPointF(), I->getNode(m+1)->position().toPointF());
                    QPointF theIntersection;
                    if (lfrom.intersect(l3, &theIntersection) == QLineF::BoundedIntersection) {
                        intersectedFrom = true;
                        QLineF lt = QLineF(nv.p2(), theIntersection);
                        if (lt.length() < lfrom.length())
                            lfrom = lt;
                    }
                }
            }
            if (intersectedFrom) {
                lfrom.setLength(lfrom.length() - STREET_NUMBERS_LENGTH);
                pfrom = lfrom.p2();

                R = new Way;
                theList->add(new AddFeatureCommand(theDocument->getDirtyOrOriginLayer(),R,true));
                theList->add(new SetTagCommand(R, "addr:interpolation", ""));
                theList->add(new SetTagCommand(R, "addr:street", streetName));

                N = new Node(Coord(pfrom));
                theList->add(new AddFeatureCommand(theDocument->getDirtyOrOriginLayer(),N,true));
                theList->add(new WayAddNodeCommand(R, N, theDocument->getDirtyOrOriginLayer(R->layer())));
                theList->add(new SetTagCommand(N, "addr:housenumber", ""));
            } else {
                pfrom = prevPoint;
            }
        }

        if (intersectedTo) {
            if (j != 0) {
                lto.setLength(lto.length() - STREET_NUMBERS_LENGTH);
                pto = lto.p2();

                N = new Node(Coord(pto));
                theList->add(new AddFeatureCommand(theDocument->getDirtyOrOriginLayer(),N,true));
                theList->add(new WayAddNodeCommand(R, N, theDocument->getDirtyOrOriginLayer(R->layer())));
                theList->add(new SetTagCommand(N, "addr:housenumber", ""));
            }
        } else {
            if (theAngle < 85. || theAngle > 95. || j== 0 || j == theRoad->size()-1) {
                N = new Node(Coord(nv.p2()));
                theList->add(new AddFeatureCommand(theDocument->getDirtyOrOriginLayer(),N,true));
                theList->add(new WayAddNodeCommand(R, N, theDocument->getDirtyOrOriginLayer(R->layer())));
                theList->add(new SetTagCommand(N, "addr:housenumber", ""));
            }

            pto = nv.p2();
        }
        prevPoint = nv.p2();
    }
}

void addStreetNumbers(Document* theDocument, CommandList* theList, PropertiesDock* theDock)
{
    QList<Way*> Roads;
    for (int i=0; i<theDock->size(); ++i)
        if (Way* R = CAST_WAY(theDock->selection(i)))
            Roads.push_back(R);

    if (Roads.isEmpty())
        return;

    QList<Way*>::const_iterator it = Roads.constBegin();
    for (;it != Roads.constEnd(); ++it) {
        if((*it)->size() < 2)
            continue;

        createStreetNumbers(theDocument, theList, (*it), false);
        createStreetNumbers(theDocument, theList, (*it), true);
    }

    if (Roads.size() == 1)
        theList->setFeature(Roads.at(0));
}

void alignNodes(Document* theDocument, CommandList* theList, PropertiesDock* theDock)
{
    if (theDock->size() < 3) //thre must be at least 3 nodes to align something
        return;

    //We build a list of selected nodes
    QList<Node*> Nodes;
    for (int i=0; i<theDock->size(); ++i)
        if (Node* N = CAST_NODE(theDock->selection(i)))
            Nodes.push_back(N);

    //we check that we have at least 3 nodes and the first two can give a line
    if(Nodes.size() < 3)
        return;
    if(Nodes[0]->position() == Nodes[1]->position())
        return;

    //we do the alignement
    Coord pos(0,0);
    const Coord p1(Nodes[0]->position());
    const Coord p2(Nodes[1]->position()-p1);
    const double slope = angle(p2);
    for (int i=2; i<Nodes.size(); ++i) {
        pos=Nodes[i]->position()-p1;
        rotate(pos,-slope);
        pos.setLat(0);
        rotate(pos,slope);
        pos=pos+p1;
        theList->add(new MoveNodeCommand( Nodes[i], pos, theDocument->getDirtyOrOriginLayer(Nodes[i]->layer()) ));
    }
}

void spreadNodes(Document* theDocument, CommandList* theList, PropertiesDock* theDock)
{
    // There must be at least 3 nodes to align something
    if (theDock->size() < 3)
        return;

    // We build a list of selected nodes
    // Sort by distance along the line between the first two nodes
    QList<Node*> Nodes;
    QList<float> Metrics;
    Coord p;
    Coord delta;
    for (int i=0; i<theDock->size(); ++i) {
        if (Node* N = CAST_NODE(theDock->selection(i))) {
            Coord pos(N->position());
            if (Nodes.size() == 0) {
                p = pos;
                Nodes.push_back(N);
                Metrics.push_back(0.0f);
            } else if (Nodes.size() == 1) {
                delta = pos - p;
                // First two nodes must form a line
                if (delta.isNull())
                    return;
                Nodes.push_back(N);
                Metrics.push_back(delta.lon()*delta.lon() + delta.lat()*delta.lat());
            } else {
                pos = pos - p;
                float metric = pos.lon()*delta.lon() + pos.lat()*delta.lat();
                // This could be done more efficiently with a binary search
                for (int j = 0; j < Metrics.size(); ++j) {
                    if (metric < Metrics[j]) {
                        Nodes.insert(j, N);
                        Metrics.insert(j, metric);
                        goto inserted;
                    }
                }
                Nodes.push_back(N);
                Metrics.push_back(metric);
inserted:
                ;
            }
        }
    }

    // We check that we have at least 3 nodes
    if(Nodes.size() < 3)
        return;

    // Do the spreading between the extremes
    p = Nodes[0]->position();
    delta = (Nodes[Nodes.size()-1]->position() - p) / (Nodes.size()-1);

    for (int i=1; i<Nodes.size()-1; ++i) {
        p = p + delta;
        theList->add(new MoveNodeCommand( Nodes[i], p, theDocument->getDirtyOrOriginLayer(Nodes[i]->layer()) ));
    }
}

static void mergeNodes(Document* theDocument, CommandList* theList, Node *node1, Node *node2)
{
    QList<Feature*> alt;
    alt.append(node1);
    Feature::mergeTags(theDocument, theList, node1, node2);
    theList->add(new RemoveFeatureCommand(theDocument, node2, alt));
}

void mergeNodes(Document* theDocument, CommandList* theList, PropertiesDock* theDock)
{
    if (theDock->size() <= 1)
        return;
    QList<Node*> Nodes;
    QList<Feature*> alt;
    for (int i=0; i<theDock->size(); ++i)
        if (Node* N = CAST_NODE(theDock->selection(i)))
            Nodes.push_back(N);
    Node* merged = Nodes[0];
    alt.push_back(merged);
    for (int i=1; i<Nodes.size(); ++i) {
        Feature::mergeTags(theDocument, theList, merged, Nodes[i]);
        theList->add(new RemoveFeatureCommand(theDocument, Nodes[i], alt));
    }
}

void detachNode(Document* theDocument, CommandList* theList, PropertiesDock* theDock)
{
    QList<Way*> Roads, Result;
    QList<Node*> Points;
    for (int i=0; i<theDock->size(); ++i)
        if (Way* R = CAST_WAY(theDock->selection(i)))
            Roads.push_back(R);
        else if (Node* Pt = CAST_NODE(theDock->selection(i)))
            Points.push_back(Pt);

    if (Roads.size() > 1 && Points.size() > 1)
        return;

    if (Roads.size() == 0 && Points.size())
    {
        for (int i=0; i<Points.size(); ++i) {
            Way * R = Way::GetSingleParentRoad(Points[i]);
            if (R)
                theList->add(new WayRemoveNodeCommand(R, Points[i],
                    theDocument->getDirtyOrOriginLayer(R)));
        }
    }

    if (Roads.size() > 1 && Points.size() == 1)
    {
        for (int i=0; i<Roads.size(); ++i) {
            if (Roads[i]->find(Points[0]) < Roads[i]->size())
                theList->add(new WayRemoveNodeCommand(Roads[i], Points[0],
                    theDocument->getDirtyOrOriginLayer(Roads[i])));
        }
    }

    if (Roads.size() == 1 && Points.size())
    {
        for (int i=0; i<Points.size(); ++i) {
            if (Roads[0]->find(Points[i]) < Roads[0]->size())
                theList->add(new WayRemoveNodeCommand(Roads[0], Points[i],
                    theDocument->getDirtyOrOriginLayer(Roads[0])));
        }
    }
}

void commitFeatures(Document* theDocument, CommandList* theList, PropertiesDock* theDock)
{
    QSet<Feature*> Features;
    QQueue<Feature*> ToAdd;

    Layer *layer = theDocument->getDirtyOrOriginLayer();

    for (int i=0; i<theDock->size(); ++i)
        if (!theDock->selection(i)->isDirty() && !theDock->selection(i)->isSpecial())
            ToAdd.enqueue(theDock->selection(i));

    while (!ToAdd.isEmpty()) {
        Feature *feature = ToAdd.dequeue();
        Features.insert(feature);
        for (int j=0; j < feature->size(); ++j) {
            Feature *member = feature->get(j);
            if (!Features.contains(member)) {
                ToAdd.enqueue(member);
            }
        }
    }

    foreach (Feature *feature, Features)
        theList->add(new AddFeatureCommand(layer,feature,true));
}

void addRelationMember(Document* theDocument, CommandList* theList, PropertiesDock* theDock)
{
    Relation* theRelation = NULL;
    QList<Feature*> Features;
    for (int i=0; i<theDock->size(); ++i)
        if ((theDock->selection(i)->getClass() == "Relation") && !theRelation)
            theRelation = CAST_RELATION(theDock->selection(i));
        else
            Features.push_back(theDock->selection(i));

    if (!(theRelation && Features.size())) return;

    for (int i=0; i<Features.size(); ++i) {
        theList->add(new RelationAddFeatureCommand(theRelation, "", Features[i], theDocument->getDirtyOrOriginLayer(theRelation->layer())));
    }
}

void removeRelationMember(Document* theDocument, CommandList* theList, PropertiesDock* theDock)
{
    Relation* theRelation = NULL;
    QList<Feature*> Features;
    for (int i=0; i<theDock->size(); ++i)
        if ((theDock->selection(i)->getClass() == "Relation") && !theRelation)
            theRelation = CAST_RELATION(theDock->selection(i));
        else
            Features.push_back(theDock->selection(i));

    if (!theRelation && Features.size() == 1)
        theRelation = Feature::GetSingleParentRelation(Features[0]);
    if (!(theRelation && Features.size())) return;

    int idx;
    for (int i=0; i<Features.size(); ++i) {
        if ((idx = theRelation->find(Features[i])) != theRelation->size())
            theList->add(new RelationRemoveFeatureCommand(theRelation, idx, theDocument->getDirtyOrOriginLayer(theRelation->layer())));
    }
}

void subdivideRoad(Document* theDocument, CommandList *theList, PropertiesDock* theDock, int divisions)
{
    // Subidviding into 1 division is no-op
    if (divisions < 2)
        return;

    // Get the selected way and nodes
    Way* theRoad = NULL;
    Node* theNodes[2] = { NULL, NULL };
    QList<Feature*> Features;
    for (int i = 0; i < theDock->size(); ++i) {
        if ((theDock->selection(i)->getClass() == "Road") && !theRoad)
            theRoad = CAST_WAY(theDock->selection(i));
        else if (theDock->selection(i)->getClass() == "TrackPoint") {
            if (!theNodes[0])
                theNodes[0] = CAST_NODE(theDock->selection(i));
            else if (!theNodes[1])
                theNodes[1] = CAST_NODE(theDock->selection(i));
        }
    }

    // If the way has only two nodes, use them
    if (theRoad && theRoad->size() == 2) {
        theNodes[0] = theRoad->getNode(0);
        theNodes[1] = theRoad->getNode(1);
        // Now this would just be silly
        if (theNodes[0] == theNodes[1])
            return;
    }

    // A way and 2 nodes
    if (!theRoad || !theNodes[0] || !theNodes[1]) {
        qDebug() << "Select a way and 2 nodes in the way";
        return;
    }

    // Nodes must be adjacent in the way
    int numNodes = theRoad->size();
    int nodeIndex0 = -1;
    for (int i = 0; i < numNodes-1; ++i) {
        Node* N0 = theRoad->getNode(i);
        Node* N1 = theRoad->getNode(i+1);
        if (N0 == theNodes[0] && N1 == theNodes[1]) {
            nodeIndex0 = i;
            break;
        } else if (N0 == theNodes[1] && N1 == theNodes[0]) {
            qSwap(theNodes[0], theNodes[1]);
            nodeIndex0 = i;
            break;
        }
    }

    if (nodeIndex0 < 0) {
        qDebug() << "Nodes are not adjacent in the way";
        return;
    }

    // Add divisions-1 new nodes in between
    Coord nodeBase = theNodes[0]->position();
    Coord nodeDelta = (theNodes[1]->position() - nodeBase) / divisions;
    for (int i = 1; i < divisions; ++i) {
        nodeBase = nodeBase + nodeDelta;
        Node* newNode = new Node(nodeBase);
        theList->add(new AddFeatureCommand(theDocument->getDirtyOrOriginLayer(theRoad), newNode, true));
        theList->add(new WayAddNodeCommand(theRoad, newNode, nodeIndex0 + i,
                                           theDocument->getDirtyOrOriginLayer(theRoad)));
        Features << newNode;
    }

    theDock->setSelection(Features);
}

void splitArea(Document* theDocument, CommandList *theList, PropertiesDock* theDock)
{
    // FIXME only enable when possible to perform (closed way selected etc)

    if (theDock->size() != 3)
        return;

    // Get the selected way and nodes
    Way* theArea = NULL;
    Node* theNodes[2] = { NULL, NULL };
    QList<Feature*> Features;
    for (int i = 0; i < theDock->size(); ++i) {
        if ((theDock->selection(i)->getClass() == "Road") && !theArea)
            theArea = CAST_WAY(theDock->selection(i));
        else if (theDock->selection(i)->getClass() == "TrackPoint") {
            if (!theNodes[0])
                theNodes[0] = CAST_NODE(theDock->selection(i));
            else if (!theNodes[1])
                theNodes[1] = CAST_NODE(theDock->selection(i));
        }
    }

    // A way and 2 nodes
    if (!theArea || !theNodes[0] || !theNodes[1]) {
        qDebug() << "Select a way and 2 nodes in the way";
        return;
    }

    // Way must be closed
    if (!theArea->isClosed()) {
        qDebug() << "Way must be closed";
        return;
    }

    // Nodes must belong to way
    int numNodes = theArea->size();
    int nodeIndex[2];
    for (int i = 0; i < 2; ++i) {
        nodeIndex[i] = theArea->find(theNodes[i]);
        if (nodeIndex[i] >= numNodes) {
            qDebug() << "Nodes must be part of way";
            return;
        }
    }

    // Make sure nodes are in order
    if (nodeIndex[0] > nodeIndex[1]) {
        qSwap(nodeIndex[0], nodeIndex[1]);
        qSwap(theNodes[0], theNodes[1]);
    }

    // And not next to one another
    if (nodeIndex[0] + 1 == nodeIndex[1] ||
            (nodeIndex[0] == 0 && nodeIndex[1] == numNodes - 2)) {
        qDebug() << "Nodes must not be adjacent";
        return;
    }

    // Extract nodes between nodeIndex[0] and nodeIndex[1] into a separate area
    // and remove the nodes from the original area
    Way* newArea = new Way;
    copyTags(newArea, theArea);
    theList->add(new AddFeatureCommand(theDocument->getDirtyOrOriginLayer(theArea), newArea, true));
    theList->add(new WayAddNodeCommand(newArea, theNodes[0], theDocument->getDirtyOrOriginLayer(theArea)));
    for (int i = nodeIndex[0]+1; i < nodeIndex[1]; ++i) {
        theList->add(new WayAddNodeCommand(newArea, theArea->getNode(nodeIndex[0]+1),
                                           theDocument->getDirtyOrOriginLayer(theArea)));
        theList->add(new WayRemoveNodeCommand(theArea, theArea->getNode(nodeIndex[0]+1),
                                              theDocument->getDirtyOrOriginLayer(theArea)));
    }
    theList->add(new WayAddNodeCommand(newArea, theNodes[1], theDocument->getDirtyOrOriginLayer(theArea)));
    theList->add(new WayAddNodeCommand(newArea, theNodes[0], theDocument->getDirtyOrOriginLayer(theArea)));
    handleWaysplitRelations(theDocument, theList, theArea, newArea);

    theDock->setSelection(QList<Way*>() << theArea << newArea);
}

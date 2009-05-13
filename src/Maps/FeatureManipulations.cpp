#include "Maps/FeatureManipulations.h"
#include "Command/DocumentCommands.h"
#include "Command/FeatureCommands.h"
#include "Command/RoadCommands.h"
#include "Command/RelationCommands.h"
#include "Command/TrackPointCommands.h"
#include "Maps/MapDocument.h"
#include "Maps/Road.h"
#include "Maps/Relation.h"
#include "Maps/TrackPoint.h"
#include "PropertiesDock.h"

#include <QtCore/QString>

#include <algorithm>

#ifndef _MOBILE
#include <geometry/geometry.hpp>
#endif

bool canJoin(Road* R1, Road* R2)
{
	if ( (R1->size() == 0) || (R2->size() == 0) )
		return true;
	MapFeature* Start1 = R1->get(0);
	MapFeature* End1 = R1->get(R1->size()-1);
	MapFeature* Start2 = R2->get(0);
	MapFeature* End2 = R2->get(R2->size()-1);
	return (Start1 == Start2) ||
		(Start1 == End2) ||
		(Start2 == End1) ||
		(End2 == End1);
}

bool canBreak(Road* R1, Road* R2)
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
	QList<Road*> Input;
	for (int i=0; i<theDock->size(); ++i)
		if (Road* R = dynamic_cast<Road*>(theDock->selection(i)))
			if (!(R->isClosed()))
				Input.push_back(R);
	for (int i=0; i<Input.size(); ++i)
		for (int j=i+1; j<Input.size(); ++j)
			if (canJoin(Input[i],Input[j]))
				return true;
	return false;
}

bool canBreakRoads(PropertiesDock* theDock)
{
	QList<Road*> Input;
	for (int i=0; i<theDock->size(); ++i)
		if (Road* R = dynamic_cast<Road*>(theDock->selection(i)))
			Input.push_back(R);
	for (int i=0; i<Input.size(); ++i)
		for (int j=i+1; j<Input.size(); ++j)
			if (canBreak(Input[i],Input[j]))
				return true;
	return false;
}

bool canDetachNodes(PropertiesDock* theDock)
{
	QList<Road*> Roads, Result;
	QList<TrackPoint*> Points;
	for (int i=0; i<theDock->size(); ++i)
		if (Road* R = dynamic_cast<Road*>(theDock->selection(i)))
			Roads.push_back(R);
		else if (TrackPoint* Pt = dynamic_cast<TrackPoint*>(theDock->selection(i)))
			Points.push_back(Pt);

	if (Roads.size() > 1 && Points.size() > 1)
		return false;

	if (Roads.size() == 0 && Points.size()) {
		for (int i=0; i<Points.size(); ++i) {
			Road * R = Road::GetSingleParentRoad(Points[i]);
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

void reversePoints(MapDocument* theDocument, CommandList* theList, Road* R)
{
	QList<TrackPoint*> Pts;
	for (int i=R->size(); i; --i)
	{
		TrackPoint* Pt = R->getNode(i-1);
		Pts.push_back(Pt);
	}
	for (int i=0; i<Pts.size(); ++i)
		theList->add(new RoadRemoveTrackPointCommand(R,Pts[i],theDocument->getDirtyOrOriginLayer(R->layer())));
	for (int i=0; i<Pts.size(); ++i)
		theList->add(new RoadAddTrackPointCommand(R,Pts[i],theDocument->getDirtyOrOriginLayer(R->layer())));
}

static void appendPoints(MapDocument* theDocument, CommandList* L, Road* Dest, Road* Src)
{
	L->add(new RoadRemoveTrackPointCommand(Src,(int)0,theDocument->getDirtyOrOriginLayer(Src->layer())));
	while (Src->size())
	{
		TrackPoint* Pt = Src->getNode(0);
		L->add(new RoadRemoveTrackPointCommand(Src,(int)0,theDocument->getDirtyOrOriginLayer(Src->layer())));
		L->add(new RoadAddTrackPointCommand(Dest,Pt,theDocument->getDirtyOrOriginLayer(Src->layer())));
	}
}

static Road* join(MapDocument* theDocument, CommandList* L, Road* R1, Road* R2)
{
	QList<MapFeature*> Alternatives;
	if (R1->size() == 0)
	{
		MapFeature::mergeTags(theDocument,L,R2,R1);
		L->add(new RemoveFeatureCommand(theDocument,R1,Alternatives));
		return R2;
	}
	if (R2->size() == 0)
	{
		MapFeature::mergeTags(theDocument,L,R1,R2);
		L->add(new RemoveFeatureCommand(theDocument,R2,Alternatives));
		return R1;
	}
	MapFeature* Start1 = R1->get(0);
	MapFeature* End1 = R1->get(R1->size()-1);
	MapFeature* Start2 = R2->get(0);
	MapFeature* End2 = R2->get(R2->size()-1);
	if ( (Start1 == Start2) || (Start1 == End2) )
		reversePoints(theDocument,L,R1);
	if ( (End1 == End2) || (Start1 == End2) )
		reversePoints(theDocument,L,R2);
	appendPoints(theDocument,L,R1,R2);
	MapFeature::mergeTags(theDocument,L,R1,R2);
	L->add(new RemoveFeatureCommand(theDocument,R2,Alternatives));
	return R1;
}

void joinRoads(MapDocument* theDocument, CommandList* theList, PropertiesDock* theDock)
{
	QList<Road*> Input;
	for (int i=0; i<theDock->size(); ++i)
		if (Road* R = dynamic_cast<Road*>(theDock->selection(i)))
			if (!(R->area() > 0.0))
				Input.push_back(R);
	while (Input.size() > 1)
	{
		int Break = true;
		for (int i=0; i<Input.size(); ++i)
			for (int j=i+1; j<Input.size(); ++j)
				if (canJoin(Input[i],Input[j]))
				{
					Road* R = join(theDocument, theList,Input[i],Input[j]);
					Input.erase(Input.begin()+j);
					Input[i] = R;
					i=j=Input.size();
					Break = false;
				}
		if (Break)
			break;
	}
	theDock->setSelection(Input);
}

static void splitRoad(MapDocument* theDocument, CommandList* theList, Road* In, const QList<TrackPoint*>& Points, QList<Road*>& Result)
{
	int pos;
	if (In->isClosed()) {  // Special case: If area, rotate the area so that the start node is the first point of splitting

		QList<TrackPoint*> Target;
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
			TrackPoint* N = new TrackPoint(*(In->getNode(pos)));
			theList->add(new AddFeatureCommand(theDocument->getDirtyOrOriginLayer(In->layer()),N,true));

			Target.prepend(N);
		} else // otherwise, just close the modified area
			Target.prepend(In->getNode(pos));

		// Now, reconstruct the road/area
		while (In->size())
			theList->add(new RoadRemoveTrackPointCommand(In,(int)0,theDocument->getDirtyOrOriginLayer(In->layer())));

		for (int i=0; i<Target.size(); ++i)
			theList->add(new RoadAddTrackPointCommand(In,Target[i],theDocument->getDirtyOrOriginLayer(In->layer())));

		if (Points.size() == 1) {  // For 1-point, we are done
			Result.push_back(In);
			return;
		}
	}

	Road* FirstPart = In;
	Result.push_back(FirstPart);
	for (int i=1; (i+1)<FirstPart->size(); ++i)
	{
		if (std::find(Points.begin(),Points.end(),FirstPart->get(i)) != Points.end())
		{
			Road* NextPart = new Road;
			copyTags(NextPart,FirstPart);
			theList->add(new AddFeatureCommand(theDocument->getDirtyOrOriginLayer(In->layer()),NextPart,true));
			theList->add(new RoadAddTrackPointCommand(NextPart, FirstPart->getNode(i), theDocument->getDirtyOrOriginLayer(In->layer())));
            for (int j=0; j < In->sizeParents(); j++) {
				Relation* L = CAST_RELATION(In->getParent(j));
				theList->add(new RelationAddFeatureCommand(L, L->getRole(L->find(In)), NextPart, theDocument->getDirtyOrOriginLayer(In->layer())));
            }
			while ( (i+1) < FirstPart->size() )
			{
				theList->add(new RoadAddTrackPointCommand(NextPart, FirstPart->getNode(i+1), theDocument->getDirtyOrOriginLayer(In->layer())));
				theList->add(new RoadRemoveTrackPointCommand(FirstPart,i+1,theDocument->getDirtyOrOriginLayer(In->layer())));
			}
			Result.push_back(NextPart);
			FirstPart = NextPart;
			i=0;
		}
	}
}

void splitRoads(MapDocument* theDocument, CommandList* theList, PropertiesDock* theDock)
{
	QList<Road*> Roads, Result;
	QList<TrackPoint*> Points;
	for (int i=0; i<theDock->size(); ++i)
		if (Road* R = dynamic_cast<Road*>(theDock->selection(i)))
			Roads.push_back(R);
		else if (TrackPoint* Pt = dynamic_cast<TrackPoint*>(theDock->selection(i)))
			Points.push_back(Pt);

	if (Roads.size() == 0 && Points.size() == 1)
	{
		Road * R = Road::GetSingleParentRoadInner(Points[0]);
		if (R)
			Roads.push_back(R);
	}

	for (int i=0; i<Roads.size(); ++i)
		splitRoad(theDocument, theList,Roads[i],Points, Result);
	theDock->setSelection(Result);
}

static void breakRoad(MapDocument* theDocument, CommandList* theList, Road* R, TrackPoint* Pt)
{
	for (int i=0; i<R->size(); ++i)
		if (R->get(i) == Pt)
		{
			TrackPoint* New = new TrackPoint(*Pt);
			copyTags(New,Pt);
			theList->add(new AddFeatureCommand(theDocument->getDirtyOrOriginLayer(R->layer()),New,true));
			theList->add(new RoadRemoveTrackPointCommand(R,i,theDocument->getDirtyOrOriginLayer(R->layer())));
			theList->add(new RoadAddTrackPointCommand(R,New,i,theDocument->getDirtyOrOriginLayer(R->layer())));
		}
		if (!Pt->sizeParents())
			theList->add(new RemoveFeatureCommand(theDocument,Pt));
}

void breakRoads(MapDocument* theDocument, CommandList* theList, PropertiesDock* theDock)
{
	QList<Road*> Roads, Result;
	QList<TrackPoint*> Points;
	for (int i=0; i<theDock->size(); ++i)
		if (Road* R = dynamic_cast<Road*>(theDock->selection(i)))
			Roads.push_back(R);
		else if (TrackPoint* Pt = dynamic_cast<TrackPoint*>(theDock->selection(i)))
			Points.push_back(Pt);

	if (Roads.size() == 0 && Points.size() == 1)
	{
		for (int i=0; i<Points[0]->sizeParents() ; ++i) {
			Road * R = dynamic_cast<Road*>(Points[0]->getParent(i));
			if (R)
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
				breakRoad(theDocument, theList, Roads[k],dynamic_cast<TrackPoint*>(Roads[i]->get(j)));
}

bool canCreateJunction(PropertiesDock* theDock)
{
	return createJunction(NULL, NULL, theDock, false);
}

bool createJunction(MapDocument* theDocument, CommandList* theList, PropertiesDock* theDock, bool doIt)
{
	//TODO test that the junction do not already exists!
	typedef geometry::point_xy<float> P;
	bool ret = false;

	QList<Road*> Roads, Result;
	for (int i=0; i<theDock->size(); ++i)
		if (Road* R = dynamic_cast<Road*>(theDock->selection(i)))
			Roads.push_back(R);

	if (Roads.size() < 2)
		return false;

	Road* R1 = Roads[0];
	Road* R2 = Roads[1];

	for (int i=0; i<R1->size()-1; ++i) {
		P a(R1->getNode(i)->position().lon(), R1->getNode(i)->position().lat());
		P b(R1->getNode(i+1)->position().lon(), R1->getNode(i+1)->position().lat());
        geometry::segment<P> s1(a, b);

		for (int j=0; j<R2->size()-1; ++j) {
			P c(R2->getNode(j)->position().lon(), R2->getNode(j)->position().lat());
			P d(R2->getNode(j+1)->position().lon(), R2->getNode(j+1)->position().lat());
			geometry::segment<P> s2(c, d);

			QList<P> theintersections;
			theintersections.append(P(0, 0));
			theintersections.append(P(0, 0));
			QList<P>::Iterator intersectOS = theintersections.begin();

			geometry::intersection_result r = geometry::intersection_segment<P>(s1, s2, intersectOS);
			if (r.is_type == geometry::is_intersect)
				if (r.get_connection_type() == geometry::is_connect_no) {
					ret = true;
					if (doIt) {
						TrackPoint* pt = new TrackPoint(Coord(theintersections[0].y(), theintersections[0].x()));
						theList->add(new AddFeatureCommand(theDocument->getDirtyOrOriginLayer(R1->layer()),pt,true));
						theList->add(new RoadAddTrackPointCommand(R1,pt,i+1,theDocument->getDirtyOrOriginLayer(R1->layer())));
						theList->add(new RoadAddTrackPointCommand(R2,pt,j+1,theDocument->getDirtyOrOriginLayer(R2->layer())));
						++i; ++j;
					} else
						return true;
				}
		}
	}
	return ret;
}

void alignNodes(MapDocument* theDocument, CommandList* theList, PropertiesDock* theDock)
{
	if (theDock->size() < 3) //thre must be at least 3 nodes to align something
		return;
	
	//We build a list of selected nodes
	QList<TrackPoint*> Nodes;
	for (int i=0; i<theDock->size(); ++i)
		if (TrackPoint* N = dynamic_cast<TrackPoint*>(theDock->selection(i)))
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
	for (int i=2; i<Nodes.size(); ++i) {
		pos=Nodes[i]->position()-p1;
		rotate(pos,-angle(p2));
		pos.setLat(0);
		rotate(pos,angle(p2));
		pos=pos+p1;
		theList->add(new MoveTrackPointCommand( Nodes[i], pos, theDocument->getDirtyOrOriginLayer(Nodes[i]->layer()) ));
	}
}

void mergeNodes(MapDocument* theDocument, CommandList* theList, PropertiesDock* theDock)
{
	if (theDock->size() <= 1)
		return;
	QList<TrackPoint*> Nodes;
	QList<MapFeature*> alt;
	for (int i=0; i<theDock->size(); ++i)
		if (TrackPoint* N = dynamic_cast<TrackPoint*>(theDock->selection(i)))
			Nodes.push_back(N);
	TrackPoint* merged = Nodes[0];
	alt.push_back(merged);
	for (int i=1; i<Nodes.size(); ++i) {
		MapFeature::mergeTags(theDocument, theList, merged, Nodes[i]);
		theList->add(new RemoveFeatureCommand(theDocument, Nodes[i], alt));
	}
}

void detachNode(MapDocument* theDocument, CommandList* theList, PropertiesDock* theDock)
{
	QList<Road*> Roads, Result;
	QList<TrackPoint*> Points;
	for (int i=0; i<theDock->size(); ++i)
		if (Road* R = dynamic_cast<Road*>(theDock->selection(i)))
			Roads.push_back(R);
		else if (TrackPoint* Pt = dynamic_cast<TrackPoint*>(theDock->selection(i)))
			Points.push_back(Pt);

	if (Roads.size() > 1 && Points.size() > 1)
		return;

	if (Roads.size() == 0 && Points.size())
	{
		for (int i=0; i<Points.size(); ++i) {
			Road * R = Road::GetSingleParentRoad(Points[i]);
			if (R)
				theList->add(new RoadRemoveTrackPointCommand(R, Points[i],
					theDocument->getDirtyOrOriginLayer(R)));
		}
	}

	if (Roads.size() > 1 && Points.size() == 1)
	{
		for (int i=0; i<Roads.size(); ++i) {
			if (Roads[i]->find(Points[0]) < Roads[i]->size())
				theList->add(new RoadRemoveTrackPointCommand(Roads[i], Points[0],
					theDocument->getDirtyOrOriginLayer(Roads[i])));
		}
	}

	if (Roads.size() == 1 && Points.size())
	{
		for (int i=0; i<Points.size(); ++i) {
			if (Roads[0]->find(Points[i]) < Roads[0]->size())
				theList->add(new RoadRemoveTrackPointCommand(Roads[0], Points[i],
					theDocument->getDirtyOrOriginLayer(Roads[0])));
		}
	}
}

void commitFeatures(MapDocument* theDocument, CommandList* theList, PropertiesDock* theDock)
{
	QList<MapFeature*> alt;
	QList<MapFeature*> Features;

	for (int i=0; i<theDock->size(); ++i)
		if (!theDock->selection(i)->layer()->isUploadable())
			Features.push_back(theDock->selection(i));
	for (int i=0; i<Features.size(); ++i) {
		if (TrackPoint* N = dynamic_cast<TrackPoint *>(Features[i])) {
			theList->add(new AddFeatureCommand(theDocument->getDirtyOrOriginLayer(),N,true));
		}
		if (Road* R = dynamic_cast<Road *>(Features[i])) {
			theList->add(new AddFeatureCommand(theDocument->getDirtyOrOriginLayer(),R,true));
			for (int j=0; j < R->size(); ++j) {
				if (!Features.contains(R->get(j))) {
					if ( !(R->get(j)->layer()->isUploadable()) ) {
						theList->add(new AddFeatureCommand(theDocument->getDirtyOrOriginLayer(),R->get(j),true));
					}
				}
			}
		}
	}
}

void addRelationMember(MapDocument* theDocument, CommandList* theList, PropertiesDock* theDock)
{
	Relation* theRelation = NULL;
	QList<MapFeature*> Features;
	for (int i=0; i<theDock->size(); ++i)
		if ((theDock->selection(i)->getClass() == "Relation") && !theRelation)
			theRelation = dynamic_cast<Relation*>(theDock->selection(i));
		else 
			Features.push_back(theDock->selection(i));

	if (!(theRelation && Features.size())) return;

	for (int i=0; i<Features.size(); ++i) {
		theList->add(new RelationAddFeatureCommand(theRelation, "", Features[i], theDocument->getDirtyOrOriginLayer(theRelation->layer()))); 
	}
}

void removeRelationMember(MapDocument* theDocument, CommandList* theList, PropertiesDock* theDock)
{
	Relation* theRelation = NULL;
	QList<MapFeature*> Features;
	for (int i=0; i<theDock->size(); ++i)
		if ((theDock->selection(i)->getClass() == "Relation") && !theRelation)
			theRelation = dynamic_cast<Relation*>(theDock->selection(i));
		else 
			Features.push_back(theDock->selection(i));

	if (!theRelation && Features.size() == 1)
		theRelation = MapFeature::GetSingleParentRelation(Features[0]);
	if (!(theRelation && Features.size())) return;

	int idx;
	for (int i=0; i<Features.size(); ++i) {
		if ((idx = theRelation->find(Features[i])) != theRelation->size())
			theList->add(new RelationRemoveFeatureCommand(theRelation, idx, theDocument->getDirtyOrOriginLayer(theRelation->layer()))); 
	}
}


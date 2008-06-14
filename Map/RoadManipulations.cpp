#include "Map/RoadManipulations.h"
#include "Command/DocumentCommands.h"
#include "Command/FeatureCommands.h"
#include "Command/RoadCommands.h"
#include "Command/RelationCommands.h"
#include "Command/TrackPointCommands.h"
#include "Map/MapDocument.h"
#include "Map/Road.h"
#include "Map/Relation.h"
#include "Map/TrackPoint.h"
#include "PropertiesDock.h"

#include <QtCore/QString>

#include <algorithm>

static bool canJoin(Road* R1, Road* R2)
{
	if ( (R1->size() == 0) || (R2->size() == 0) )
		return true;
	TrackPoint* Start1 = R1->get(0);
	TrackPoint* End1 = R1->get(R1->size()-1);
	TrackPoint* Start2 = R2->get(0);
	TrackPoint* End2 = R2->get(R2->size()-1);
	return (Start1 == Start2) ||
		(Start1 == End2) ||
		(Start2 == End1) ||
		(End2 == End1);
}

static void mergeTags(MapLayer* theLayer, CommandList* L, MapFeature* Dest, MapFeature* Src)
{
	for (unsigned int i=0; i<Src->tagSize(); ++i)
	{
		QString k = Src->tagKey(i);
		QString v1 = Src->tagValue(i);
		unsigned int j = Dest->findKey(k);
		if (j == Dest->tagSize())
			L->add(new SetTagCommand(Dest,k,v1, theLayer));
		else
		{
			QString v2 = Dest->tagValue(j);
			if (v1 != v2 && k !="created_by")
			{
				L->add(new SetTagCommand(Dest,k,QString("%1;%2").arg(v2).arg(v1), theLayer));
			}
		}
	}
}

void reversePoints(MapLayer* theLayer, CommandList* theList, Road* R)
{
	std::vector<TrackPoint*> Pts;
	for (unsigned int i=R->size(); i; --i)
	{
		TrackPoint* Pt = R->get(i-1);
		Pts.push_back(Pt);
	}
	for (unsigned int i=0; i<Pts.size(); ++i)
		theList->add(new RoadRemoveTrackPointCommand(R,Pts[i],theLayer));
	for (unsigned int i=0; i<Pts.size(); ++i)
		theList->add(new RoadAddTrackPointCommand(R,Pts[i],theLayer));
}

static void appendPoints(MapLayer* theLayer, CommandList* L, Road* Dest, Road* Src)
{
	L->add(new RoadRemoveTrackPointCommand(Src,(unsigned int)0,theLayer));
	while (Src->size())
	{
		TrackPoint* Pt = Src->get(0);
		L->add(new RoadRemoveTrackPointCommand(Src,(unsigned int)0,theLayer));
		L->add(new RoadAddTrackPointCommand(Dest,Pt,theLayer));
	}
}

static Road* join(MapDocument* theDocument, CommandList* L, Road* R1, Road* R2)
{
	if (R1->size() == 0)
	{
		mergeTags(theDocument->getDirtyLayer(),L,R2,R1);
		L->add(new RemoveFeatureCommand(theDocument,R1));
		return R2;
	}
	if (R2->size() == 0)
	{
		mergeTags(theDocument->getDirtyLayer(),L,R1,R2);
		L->add(new RemoveFeatureCommand(theDocument,R2));
		return R1;
	}
	TrackPoint* Start1 = R1->get(0);
	TrackPoint* End1 = R1->get(R1->size()-1);
	TrackPoint* Start2 = R2->get(0);
	TrackPoint* End2 = R2->get(R2->size()-1);
	if ( (Start1 == Start2) || (Start1 == End2) )
		reversePoints(theDocument->getDirtyLayer(),L,R1);
	if ( (End1 == End2) || (Start1 == End2) )
		reversePoints(theDocument->getDirtyLayer(),L,R2);
	appendPoints(theDocument->getDirtyLayer(),L,R1,R2);
	mergeTags(theDocument->getDirtyLayer(),L,R1,R2);
	L->add(new RemoveFeatureCommand(theDocument,R2));
	return R1;
}

void joinRoads(MapDocument* theDocument, CommandList* theList, PropertiesDock* theDock)
{
	std::vector<Road*> Input;
	for (unsigned int i=0; i<theDock->size(); ++i)
		if (Road* R = dynamic_cast<Road*>(theDock->selection(i)))
			if (!isClosed(R))
				Input.push_back(R);
	while (Input.size() > 1)
	{
		unsigned int Break = true;
		for (unsigned int i=0; i<Input.size(); ++i)
			for (unsigned int j=i+1; j<Input.size(); ++j)
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

static void splitRoad(MapLayer* theLayer, CommandList* theList, Road* In, const std::vector<TrackPoint*>& Points, std::vector<Road*>& Result)
{
	bool WasClosed = isClosed(In);
	Road* FirstPart = In;
	Result.push_back(FirstPart);
	for (unsigned int i=1; (i+1)<FirstPart->size(); ++i)
	{
		if (std::find(Points.begin(),Points.end(),FirstPart->get(i)) != Points.end())
		{
			Road* NextPart = new Road;
			copyTags(NextPart,FirstPart);
			NextPart->add(FirstPart->get(i));
			while ( (i+1) < FirstPart->size() )
			{
				NextPart->add(FirstPart->get(i+1));
				theList->add(new RoadRemoveTrackPointCommand(FirstPart,i+1,theLayer));
			}
			if (In != FirstPart)
			{
				theList->add(new AddFeatureCommand(theLayer,FirstPart,true));
				Result.push_back(FirstPart);
			}
			FirstPart = NextPart;
			i=0;
		}
	}

	if (FirstPart != In)
	{
		if (WasClosed)
		{
			std::vector<TrackPoint*> Target;

			for (unsigned int i=0; i<FirstPart->size(); ++i)
				Target.push_back(FirstPart->get(i));

			for (unsigned int i=1; i<In->size(); ++i)
				Target.push_back(In->get(i));

			while (FirstPart->size())
				theList->add(new RoadRemoveTrackPointCommand(FirstPart,(unsigned int)0,theLayer));

			while (In->size())
				theList->add(new RoadRemoveTrackPointCommand(In,(unsigned int)0,theLayer));
			
			delete FirstPart;

			for (unsigned int i=0; i<Target.size(); ++i)
				theList->add(new RoadAddTrackPointCommand(In,Target[i],theLayer));
		}
		else
		{
			theList->add(new AddFeatureCommand(theLayer,FirstPart,true));
			Result.push_back(FirstPart);
		}
	}
}

static Road * GetSingleParentRoad(MapFeature * mapFeature)
{
	unsigned int parents = mapFeature->sizeParents();

	if (parents == 0)
		return NULL;

	Road * parentRoad = NULL;

	unsigned int i;
	for (i=0; i<parents; i++)
	{
		MapFeature * parent = mapFeature->getParent(i);
		Road * road = dynamic_cast<Road*>(parent);

		if (road == NULL)
			continue;

		if (parentRoad)
			return NULL;

		parentRoad = road;
	}

	return parentRoad;
}

void splitRoads(MapLayer* theLayer, CommandList* theList, PropertiesDock* theDock)
{
	std::vector<Road*> Roads, Result;
	std::vector<TrackPoint*> Points;
	for (unsigned int i=0; i<theDock->size(); ++i)
		if (Road* R = dynamic_cast<Road*>(theDock->selection(i)))
			Roads.push_back(R);
		else if (TrackPoint* Pt = dynamic_cast<TrackPoint*>(theDock->selection(i)))
			Points.push_back(Pt);

	if (Roads.size() == 0 && Points.size() == 1)
	{
		Road * R = GetSingleParentRoad(Points[0]);
		if (R)
			Roads.push_back(R);
	}

	for (unsigned int i=0; i<Roads.size(); ++i)
		splitRoad(theLayer, theList,Roads[i],Points, Result);
	theDock->setSelection(Result);
}

static void breakRoad(MapLayer* theLayer, CommandList* theList, Road* R, TrackPoint* Pt)
{
	for (unsigned int i=0; i<R->size(); ++i)
		if (R->get(i) == Pt)
		{
			TrackPoint* New = new TrackPoint(*Pt);
			theList->add(new AddFeatureCommand(theLayer,New,true));
			theList->add(new RoadRemoveTrackPointCommand(R,i,theLayer));
			theList->add(new RoadAddTrackPointCommand(R,New,i,theLayer));
		}
}

void breakRoads(MapLayer* theLayer, CommandList* theList, PropertiesDock* theDock)
{
	std::vector<Road*> Roads;
	for (unsigned int i=0; i<theDock->size(); ++i)
		if (Road* R = dynamic_cast<Road*>(theDock->selection(i)))
			Roads.push_back(R);
	for (unsigned int i=0; i<Roads.size(); ++i)
		for (unsigned int j=0; j<Roads[i]->size(); ++j)
			for (unsigned int k=i+1; k<Roads.size(); ++k)
				breakRoad(theLayer, theList, Roads[k],Roads[i]->get(j));
}

void alignNodes(MapLayer* theLayer, CommandList* theList, PropertiesDock* theDock)
{
	if (theDock->size() < 3) //thre must be at least 3 nodes to align something
		return;
	
	//We build a list of selected nodes
	std::vector<TrackPoint*> Nodes;
	for (unsigned int i=0; i<theDock->size(); ++i)
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
	for (unsigned int i=2; i<Nodes.size(); ++i) {
		pos=Nodes[i]->position()-p1;
		rotate(pos,-angle(p2));
		pos.setLat(0);
		rotate(pos,angle(p2));
		pos=pos+p1;
		theList->add(new MoveTrackPointCommand( Nodes[i], pos, theLayer ));
	}
}

void mergeNodes(MapDocument* theDocument, CommandList* theList, PropertiesDock* theDock)
{
	if (theDock->size() <= 1)
		return;
	std::vector<TrackPoint*> Nodes;
	std::vector<MapFeature*> alt;
	for (unsigned int i=0; i<theDock->size(); ++i)
		if (TrackPoint* N = dynamic_cast<TrackPoint*>(theDock->selection(i)))
			Nodes.push_back(N);
	TrackPoint* merged = Nodes[0];
	alt.push_back(merged);
	for (unsigned int i=1; i<Nodes.size(); ++i) {
		mergeTags(theDocument->getDirtyLayer(), theList, merged, Nodes[i]);
		theList->add(new RemoveFeatureCommand(theDocument, Nodes[i], alt));
	}
}


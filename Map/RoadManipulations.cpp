#include "Map/RoadManipulations.h"
#include "Command/DocumentCommands.h"
#include "Command/FeatureCommands.h"
#include "Command/RoadCommands.h"
#include "Command/RelationCommands.h"
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

static void mergeTags(CommandList* L, MapFeature* Dest, MapFeature* Src)
{
	for (unsigned int i=0; i<Src->tagSize(); ++i)
	{
		QString k = Src->tagKey(i);
		QString v1 = Src->tagValue(i);
		unsigned int j = Dest->findKey(k);
		if (j == Dest->tagSize())
			L->add(new SetTagCommand(Dest,k,v1));
		else
		{
			QString v2 = Dest->tagValue(j);
			if (v1 != v2)
			{
				for (unsigned int t=1; true; t++)
				{
					QString Prop = QString("%1_alt_%2").arg(k).arg(t);
					if (Dest->findKey(Prop) == Dest->tagSize())
					{
						L->add(new SetTagCommand(Dest,Prop,v1));
						break;
					}
				}
			}
		}
	}
}

void reversePoints(CommandList* theList, Road* R)
{
	std::vector<TrackPoint*> Pts;
	for (unsigned int i=R->size(); i; --i)
	{
		TrackPoint* Pt = R->get(i-1);
		Pts.push_back(Pt);
		theList->add(new RoadRemoveTrackPointCommand(R,Pt));
	}
	for (unsigned int i=0; i<Pts.size(); ++i)
		theList->add(new RoadAddTrackPointCommand(R,Pts[i]));
}

static void appendPoints(CommandList* L, Road* Dest, Road* Src)
{
	L->add(new RoadRemoveTrackPointCommand(Src,(unsigned int)0));
	while (Src->size())
	{
		TrackPoint* Pt = Src->get(0);
		L->add(new RoadRemoveTrackPointCommand(Src,(unsigned int)0));
		L->add(new RoadAddTrackPointCommand(Dest,Pt));
	}
}

static Road* join(MapDocument* theDocument, CommandList* L, Road* R1, Road* R2)
{
	if (R1->size() == 0)
	{
		mergeTags(L,R2,R1);
		L->add(new RemoveFeatureCommand(theDocument,R1));
		return R2;
	}
	if (R2->size() == 0)
	{
		mergeTags(L,R1,R2);
		L->add(new RemoveFeatureCommand(theDocument,R2));
		return R1;
	}
	TrackPoint* Start1 = R1->get(0);
	TrackPoint* End1 = R1->get(R1->size()-1);
	TrackPoint* Start2 = R2->get(0);
	TrackPoint* End2 = R2->get(R2->size()-1);
	if ( (Start1 == Start2) || (Start1 == End2) )
		reversePoints(L,R1);
	if ( (End1 == End2) || (Start1 == End2) )
		reversePoints(L,R2);
	appendPoints(L,R1,R2);
	mergeTags(L,R1,R2);
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
				theList->add(new RoadRemoveTrackPointCommand(FirstPart,i+1));
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
			delete FirstPart;
			for (unsigned int i=1; i<In->size(); ++i)
				Target.push_back(In->get(i));
			while (In->size())
				theList->add(new RoadRemoveTrackPointCommand(In,(unsigned int)0));
			for (unsigned int i=0; i<Target.size(); ++i)
				theList->add(new RoadAddTrackPointCommand(In,Target[i]));
		}
		else
		{
			theList->add(new AddFeatureCommand(theLayer,FirstPart,true));
			Result.push_back(FirstPart);
		}
	}
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
			theList->add(new RoadRemoveTrackPointCommand(R,i));
			theList->add(new RoadAddTrackPointCommand(R,New,i));
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
		mergeTags(theList, merged, Nodes[i]);
		theList->add(new RemoveFeatureCommand(theDocument, Nodes[i], alt));
	}
}


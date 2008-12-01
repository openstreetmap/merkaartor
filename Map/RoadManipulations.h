#ifndef MERKAARTOR_ROADMANIPULATIONS_H_
#define MERKAARTOR_ROADMANIPULATIONS_H_

class CommandList;
class MapDocument;
class MapLayer;
class PropertiesDock;
class Road;

#include <vector>

void joinRoads(MapDocument* theDocument, CommandList* theList, PropertiesDock* theDock);
void splitRoads(MapDocument* theDocument, CommandList* theList, PropertiesDock* theDock);
void breakRoads(MapDocument* theDocument, CommandList* theList, PropertiesDock* theDock);
void reversePoints(MapDocument* theDocument, CommandList* theList, Road* R);
void alignNodes(MapDocument* theDocument, CommandList* theList, PropertiesDock* theDock);
void mergeNodes(MapDocument* theDocument, CommandList* theList, PropertiesDock* theDock);
void commitFeatures(MapDocument* theDocument, CommandList* theList, PropertiesDock* theDock);
bool canJoinRoads(PropertiesDock* theDock);
bool canBreakRoads(PropertiesDock* theDock);
void addRelationMember(MapDocument* theDocument, CommandList* theList, PropertiesDock* theDock);
void removeRelationMember(MapDocument* theDocument, CommandList* theList, PropertiesDock* theDock);


#endif


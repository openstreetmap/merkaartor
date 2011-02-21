#ifndef MERKAARTOR_FEATUREMANIPULATIONS_H_
#define MERKAARTOR_FEATUREMANIPULATIONS_H_

class CommandList;
class Document;
class Layer;
class PropertiesDock;
class Way;
class Projection;

#include <QList>
#include "Coord.h"

void joinRoads(Document* theDocument, CommandList* theList, PropertiesDock* theDock);
void splitRoads(Document* theDocument, CommandList* theList, PropertiesDock* theDock);
void breakRoads(Document* theDocument, CommandList* theList, PropertiesDock* theDock);
bool canCreateJunction(PropertiesDock* theDock);
int createJunction(Document* theDocument, CommandList* theList, PropertiesDock* theDock, bool doIt=true);
void addStreetNumbers(Document* theDocument, CommandList* theList, PropertiesDock* theDock);
void reversePoints(Document* theDocument, CommandList* theList, Way* R);
void simplifyRoads(Document* theDocument, CommandList* theList, PropertiesDock* theDock, qreal threshold);
void alignNodes(Document* theDocument, CommandList* theList, PropertiesDock* theDock);
void bingExtract(Document* theDocument, CommandList* theList, PropertiesDock* theDock, CoordBox vp);
void spreadNodes(Document* theDocument, CommandList* theList, PropertiesDock* theDock);
void mergeNodes(Document* theDocument, CommandList* theList, PropertiesDock* theDock);
void detachNode(Document* theDocument, CommandList* theList, PropertiesDock* theDock);
void commitFeatures(Document* theDocument, CommandList* theList, PropertiesDock* theDock);
bool canJoinRoads(PropertiesDock* theDock);
bool canBreakRoads(PropertiesDock* theDock);
bool canDetachNodes(PropertiesDock* theDock);
void addRelationMember(Document* theDocument, CommandList* theList, PropertiesDock* theDock);
void removeRelationMember(Document* theDocument, CommandList* theList, PropertiesDock* theDock);
void addToMultipolygon(Document* theDocument, CommandList* theList, PropertiesDock* theDock);
bool canSubdivideRoad(PropertiesDock* theDock, Way** theRoad = 0, unsigned int* edge = 0);
void subdivideRoad(Document* theDocument, CommandList* theList, PropertiesDock* theDock, unsigned int divisions);
void joinAreas(Document* theDocument, CommandList* theList, PropertiesDock* theDock);
bool canSplitArea(PropertiesDock* theDock, Way** outTheArea = 0, unsigned int outNodes[2] = 0);
void splitArea(Document* theDocument, CommandList* theList, PropertiesDock* theDock);
bool canTerraceArea(PropertiesDock* theDock, Way** outTheArea = 0, int* startNode = 0);
void terraceArea(Document* theDocument, CommandList* theList, PropertiesDock* theDock, unsigned int divisions);

enum AxisAlignResult {
    AxisAlignSuccess,
    // failures:
    AxisAlignSharpAngles,
    AxisAlignFail       // no convergence
};
bool canAxisAlignRoads(PropertiesDock* theDock);
unsigned int axisAlignGuessAxes(PropertiesDock* theDock, const Projection &proj, unsigned int max_axes);
AxisAlignResult axisAlignRoads(Document* theDocument, CommandList* theList, PropertiesDock* theDock, const Projection &proj, unsigned int axes);


#endif


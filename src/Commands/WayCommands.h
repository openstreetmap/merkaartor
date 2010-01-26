#ifndef MERKAARTOR_WAYOMMANDS_H_
#define MERKAARTOR_WAYOMMANDS_H_

#include "Command.h"

class Way;
class Node;
class Layer;

class WayAddNodeCommand : public Command
{
	public:
		WayAddNodeCommand(Way* R = NULL);
		WayAddNodeCommand(Way* R, Node* W, Layer* aLayer=NULL);
		WayAddNodeCommand(Way* R, Node* W, int Position, Layer* aLayer=NULL);
		~WayAddNodeCommand(void);

		virtual void undo();
		virtual void redo();
		virtual bool buildDirtyList(DirtyList& theList);

		virtual bool toXML(QDomElement& xParent) const;
		static WayAddNodeCommand* fromXML(Document* d,QDomElement e);

	private:
		Layer* theLayer;
		Layer* oldLayer;
		Way* theRoad;
		Node* theTrackPoint;
		int Position;
};

class WayRemoveNodeCommand : public Command
{
	public:
		WayRemoveNodeCommand(Way* R = NULL);
		WayRemoveNodeCommand(Way* R, Node* W, Layer* aLayer=NULL);
		WayRemoveNodeCommand(Way* R, int anIdx, Layer* aLayer=NULL);
		~WayRemoveNodeCommand(void);

		virtual void undo();
		virtual void redo();
		virtual bool buildDirtyList(DirtyList& theList);

		virtual bool toXML(QDomElement& xParent) const;
		static WayRemoveNodeCommand* fromXML(Document* d,QDomElement e);

	private:
		Layer* theLayer;
		Layer* oldLayer;
		int Idx;
		Way* theRoad;
		Node* theNode;
};

#endif



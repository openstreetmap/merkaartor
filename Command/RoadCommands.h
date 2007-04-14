#ifndef MERKAARTOR_ROADCOMMANDS_H_
#define MERKAARTOR_ROADCOMMANDS_H_

#include "Command/Command.h"

class Road;
class Way;

class RoadAddWayCommand : public Command
{
	public:
		RoadAddWayCommand(Road* R, Way* W);
		RoadAddWayCommand(Road* R, Way* W, unsigned int Position);

		virtual void undo();
		virtual void redo();
		virtual bool buildDirtyList(DirtyList& theList);

	private:
		Road* theRoad;
		Way* theWay;
		unsigned int Position;
};

class RoadRemoveWayCommand : public Command
{
	public:
		RoadRemoveWayCommand(Road* R, Way* W);

		virtual void undo();
		virtual void redo();
		virtual bool buildDirtyList(DirtyList& theList);

	private:
		unsigned int Idx;
		Road* theRoad;
		Way* theWay;
};

#endif



#ifndef MERKATOR_WAYCOMMAND_H_
#define MERKATOR_WAYCOMMAND_H_

#include "Command/Command.h"

class TrackPoint;
class Way;

class WaySetWidthCommand :	public Command
{
	public:
		WaySetWidthCommand(Way* W, double w);
		virtual ~WaySetWidthCommand();

		virtual void redo();
		virtual void undo();
		bool buildDirtyList(DirtyList& theList);

	private:
		Command* Sub;
};

class WaySetFromToCommand : public Command
{
	public:
		WaySetFromToCommand(Way* W, TrackPoint* aFrom, TrackPoint* aTo);
		WaySetFromToCommand(Way* W, TrackPoint* aFrom, TrackPoint* aC1, TrackPoint* aC2, TrackPoint* aTo);

		virtual void redo();
		virtual void undo();
		bool buildDirtyList(DirtyList& theList);
	private:
		Way* theWay;
		TrackPoint* OldFrom, *OldTo;
		TrackPoint* NewFrom, *NewTo;
		TrackPoint* OldControlFrom, *OldControlTo;
		TrackPoint* NewControlFrom, *NewControlTo;
};

#endif



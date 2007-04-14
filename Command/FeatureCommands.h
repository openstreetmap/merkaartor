#ifndef MERKAARTOR_FEATURECOMMANDS_H_
#define MERKAARTOR_FEATURECOMMANDS_H_

#include "Command/Command.h"

#include <QtCore/QString>

#include <utility>
#include <vector>

class MapFeature;

class SetTagCommand :	public Command
{
	public:
		SetTagCommand(MapFeature* aF, unsigned int idx, const QString& k, const QString& v);
		SetTagCommand(MapFeature* aF, const QString& k, const QString& v);

		virtual void undo();
		virtual void redo();
		virtual bool buildDirtyList(DirtyList& theList);

	private:
		MapFeature* theFeature;
		unsigned int Idx;
		QString Key, Value, OldKey, OldValue;
		bool Remove;
};

class ClearTagsCommand : public Command
{
	public:
		ClearTagsCommand(MapFeature* aF);

		virtual void undo();
		virtual void redo();
		virtual bool buildDirtyList(DirtyList& theList);
	private:
		MapFeature* theFeature;
		std::vector<std::pair<QString, QString> > Tags;
};

class ClearTagCommand : public Command
{
	public:
		ClearTagCommand(MapFeature* aF, const QString& k);

		virtual void undo();
		virtual void redo();
		virtual bool buildDirtyList(DirtyList& theList);
	private:
		MapFeature* theFeature;
		bool Existed;
		QString Key,Value;
};

#endif




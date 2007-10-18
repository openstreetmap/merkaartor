#ifndef MERKAARTOR_FEATURECOMMANDS_H_
#define MERKAARTOR_FEATURECOMMANDS_H_

#include "Command/Command.h"

#include <QtCore/QString>

#include <utility>
#include <vector>

class MapFeature;

class TagCommand : public Command
{
	public:
		TagCommand(MapFeature* theFeature);

		virtual void undo();
		virtual void redo();
		virtual bool buildDirtyList(DirtyList& theList);

	private:
		MapFeature* theFeature;
		std::vector<std::pair<QString, QString> > Before, After;
		bool FirstRun;
};

class SetTagCommand : public TagCommand
{
	public:
		SetTagCommand(MapFeature* aF, unsigned int idx, const QString& k, const QString& v);
		SetTagCommand(MapFeature* aF, const QString& k, const QString& v);
};

class ClearTagsCommand : public TagCommand
{
	public:
		ClearTagsCommand(MapFeature* aF);
};

class ClearTagCommand : public TagCommand
{
	public:
		ClearTagCommand(MapFeature* aF, const QString& k);
};

#endif




#ifndef MERKAARTOR_FEATURECOMMANDS_H_
#define MERKAARTOR_FEATURECOMMANDS_H_

#include "Command/Command.h"

#include <QtCore/QString>

#include <utility>
#include <vector>

class MapFeature;
class MapDocument;

class TagCommand : public Command
{
	public:
		TagCommand(MapFeature* theFeature);
		TagCommand() {};

		virtual void undo() = 0;
		virtual void redo() = 0;
		virtual bool buildDirtyList(DirtyList& theList);

	protected:
		MapFeature* theFeature;
		std::vector<std::pair<QString, QString> > Before, After;
		bool FirstRun;
};

class SetTagCommand : public TagCommand
{
	public:
		SetTagCommand() {};
		SetTagCommand(MapFeature* aF, unsigned int idx, const QString& k, const QString& v);
		SetTagCommand(MapFeature* aF, const QString& k, const QString& v);

		virtual void undo();
		virtual void redo();

		virtual bool toXML(QDomElement& xParent) const;
		static SetTagCommand* fromXML(MapDocument* d,QDomElement e);

	private:
		int theIdx;
		QString theK;
		QString theV;
};

//class ClearTagsCommand : public TagCommand
//{
//	public:
//		ClearTagsCommand() {};
//		ClearTagsCommand(MapFeature* aF);
//
//		virtual void undo();
//		virtual void redo();
//
//		virtual bool toXML(QDomElement& xParent) const;
//		static ClearTagsCommand* fromXML(MapDocument* d,QDomElement e);
//};
//
class ClearTagCommand : public TagCommand
{
	public:
		ClearTagCommand() {};
		ClearTagCommand(MapFeature* aF, const QString& k);

		virtual void undo();
		virtual void redo();

		virtual bool toXML(QDomElement& xParent) const;
		static ClearTagCommand* fromXML(MapDocument* d,QDomElement e);

	private:
		int theIdx;
		QString theK;
		QString theV;

};

#endif




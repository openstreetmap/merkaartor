#ifndef MERKAARTOR_FEATURECOMMANDS_H_
#define MERKAARTOR_FEATURECOMMANDS_H_

#include "Command/Command.h"

#include <QtCore/QString>

#include <utility>
#include <vector>

class MapFeature;
class MapDocument;
class MapLayer;

class TagCommand : public Command
{
	public:
		TagCommand(MapFeature* aF, MapLayer* aLayer);
		TagCommand(MapFeature* aF);
		~TagCommand(void);

		virtual void undo() = 0;
		virtual void redo() = 0;
		virtual bool buildDirtyList(DirtyList& theList);

	protected:
		MapFeature* theFeature;
		std::vector<std::pair<QString, QString> > Before, After;
		bool FirstRun;
		MapLayer* theLayer;
		MapLayer* oldLayer;
};

class SetTagCommand : public TagCommand
{
	public:
		SetTagCommand(MapFeature* aF);
		SetTagCommand(MapFeature* aF, unsigned int idx, const QString& k, const QString& v, MapLayer* aLayer=NULL);
		SetTagCommand(MapFeature* aF, const QString& k, const QString& v, MapLayer* aLayer=NULL);

		virtual void undo();
		virtual void redo();
		virtual bool buildDirtyList(DirtyList& theList);

		virtual bool toXML(QDomElement& xParent) const;
		static SetTagCommand* fromXML(MapDocument* d,QDomElement e);

	private:
		int theIdx;
		QString theK;
		QString theV;
		QString oldK;
		QString oldV;
};

class ClearTagsCommand : public TagCommand
{
	public:
		ClearTagsCommand(MapFeature* aF, MapLayer* aLayer=NULL);

		virtual void undo();
		virtual void redo();

		virtual bool toXML(QDomElement& xParent) const;
		static ClearTagsCommand* fromXML(MapDocument* d,QDomElement e);
};

class ClearTagCommand : public TagCommand
{
	public:
		ClearTagCommand(MapFeature* aF);
		ClearTagCommand(MapFeature* aF, const QString& k, MapLayer* aLayer=NULL);

		virtual void undo();
		virtual void redo();
		virtual bool buildDirtyList(DirtyList& theList);

		virtual bool toXML(QDomElement& xParent) const;
		static ClearTagCommand* fromXML(MapDocument* d,QDomElement e);

	private:
		int theIdx;
		QString theK;
		QString theV;

};

#endif




#ifndef MERKAARTOR_FEATURECOMMANDS_H_
#define MERKAARTOR_FEATURECOMMANDS_H_

#include "Command.h"

#include <QtCore/QString>

#include <utility>
#include <QList>

class Feature;
class Document;
class Layer;

class TagCommand : public Command
{
	public:
		TagCommand(Feature* aF, Layer* aLayer);
		TagCommand(Feature* aF);
		~TagCommand(void);

		virtual void undo() = 0;
		virtual void redo() = 0;
		virtual bool buildDirtyList(DirtyList& theList);

	protected:
		Feature* theFeature;
		QList<QPair<QString, QString> > Before, After;
		bool FirstRun;
		Layer* theLayer;
		Layer* oldLayer;
};

class SetTagCommand : public TagCommand
{
	public:
		SetTagCommand(Feature* aF);
		SetTagCommand(Feature* aF, int idx, const QString& k, const QString& v, Layer* aLayer=NULL);
		SetTagCommand(Feature* aF, const QString& k, const QString& v, Layer* aLayer=NULL);

		virtual void undo();
		virtual void redo();
		virtual bool buildDirtyList(DirtyList& theList);

		virtual bool toXML(QDomElement& xParent) const;
		static SetTagCommand* fromXML(Document* d,QDomElement e);

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
		ClearTagsCommand(Feature* aF, Layer* aLayer=NULL);

		virtual void undo();
		virtual void redo();

		virtual bool toXML(QDomElement& xParent) const;
		static ClearTagsCommand* fromXML(Document* d,QDomElement e);
};

class ClearTagCommand : public TagCommand
{
	public:
		ClearTagCommand(Feature* aF);
		ClearTagCommand(Feature* aF, const QString& k, Layer* aLayer=NULL);

		virtual void undo();
		virtual void redo();
		virtual bool buildDirtyList(DirtyList& theList);

		virtual bool toXML(QDomElement& xParent) const;
		static ClearTagCommand* fromXML(Document* d,QDomElement e);

	private:
		int theIdx;
		QString theK;
		QString theV;

};

#endif




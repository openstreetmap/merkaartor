#ifndef MERKATOR_DOCUMENTCOMMANDS_H_
#define MERKATOR_DOCUMENTCOMMANDS_H_

#include "Command.h"

#include <QList>

class Document;
class Layer;
class Feature;

class AddFeatureCommand : public Command
{
	public:
		AddFeatureCommand(Feature* aFeature = NULL);
		AddFeatureCommand(Layer* aDocument, Feature* aFeature, bool aUserAdded);
		virtual ~AddFeatureCommand();

		void undo();
		void redo();
		bool buildDirtyList(DirtyList& theList);

		virtual bool toXML(QDomElement& xParent) const;
		static AddFeatureCommand* fromXML(Document* d,QDomElement e);

	private:
		Layer* theLayer;
		Layer* oldLayer;
		Feature* theFeature;
		bool UserAdded;
};

class RemoveFeatureCommand : public Command
{
	public:
		RemoveFeatureCommand(Feature* aFeature = NULL);
		RemoveFeatureCommand(Document* theDocument, Feature* aFeature);
		RemoveFeatureCommand(Document* theDocument, Feature* aFeature, const QList<Feature*>& Alternatives);
		virtual ~RemoveFeatureCommand();

		void undo();
		void redo();
		bool buildDirtyList(DirtyList& theList);

		virtual bool toXML(QDomElement& xParent) const;
		static RemoveFeatureCommand* fromXML(Document* d,QDomElement e);

	private:
		Layer* theLayer;
		Layer* oldLayer;
		int Idx;
		Feature* theFeature;
		CommandList* CascadedCleanUp;
		bool RemoveExecuted;
		QList<Feature*> theAlternatives;
};

#endif



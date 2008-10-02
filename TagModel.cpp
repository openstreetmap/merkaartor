#include "TagModel.h"
#include "MainWindow.h"
#include "Command/DocumentCommands.h"
#include "Command/FeatureCommands.h"
#include "Map/MapDocument.h"
#include "Map/MapFeature.h"
#include "Map/MapLayer.h"

TagModel::TagModel(MainWindow* aMain)
: Main(aMain)
{
}

TagModel::~TagModel(void)
{
}

void TagModel::setFeature(const std::vector<MapFeature*> Features)
{
	if (theFeatures.size())
	{
		beginRemoveRows(QModelIndex(),0,Tags.size());
		Tags.clear();
		endRemoveRows();
	}
	theFeatures = Features;
	if (theFeatures.size())
	{
		MapFeature* F = theFeatures[0];
		for (unsigned int i=0; i<F->tagSize(); ++i)
		{
			unsigned int j=0;
			for (j=1; j<theFeatures.size(); ++j)
				if (F->tagValue(i) != theFeatures[j]->tagValue(F->tagKey(i),""))
					break;
			if (j == theFeatures.size())
				if (!F->tagKey(i).startsWith("%kml:"))
					Tags.push_back(std::make_pair(F->tagKey(i),F->tagValue(i)));
		}
		beginInsertRows(QModelIndex(),0,Tags.size());
		endInsertRows();
	}
}

int TagModel::rowCount(const QModelIndex &) const
{
	if (!theFeatures.size()) return 0;
	return Tags.size()+1;
}

int TagModel::columnCount(const QModelIndex &) const
{
	return 2;
}

QVariant TagModel::data(const QModelIndex &index, int role) const
{
	if (!theFeatures.size())
		return QVariant();
	if (!index.isValid())
		return QVariant();
	if ((unsigned int)index.row() > Tags.size())
		return QVariant();
	if (role == Qt::DisplayRole)
	{
		if ((unsigned int)index.row() >= Tags.size())
		{
			if (index.column() == 0)
				return newKeyText();
			else
				return "";
		}
		else
		{
			if (index.column() == 0)
				return Tags[index.row()].first;
			else
				return Tags[index.row()].second;
		}
	}
	else if (role == Qt::EditRole)
	{
		if ((unsigned int)index.row() >= Tags.size())
			return "";
		else
		{
			if (index.column() == 0)
				return Tags[index.row()].first;
			else
				return Tags[index.row()].second;
		}
	}
	return QVariant();
}

QVariant TagModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();
	if (orientation == Qt::Horizontal)
	{
		if (section == 0)
			return tr("Key");
		else
			return tr("Value");
	}
	return QVariant();
}

Qt::ItemFlags TagModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled;
	return QAbstractTableModel::flags(index) | Qt::ItemIsEditable  | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool TagModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!theFeatures.size()) return false;
	if (index.isValid() && role == Qt::EditRole)
	{
		if ((unsigned int)index.row() == Tags.size())
		{
			if (index.column() == 0)
			{
				beginInsertRows(QModelIndex(), Tags.size()+1, Tags.size()+1);
				CommandList* L = new CommandList(MainWindow::tr("Set Tags on %1").arg(theFeatures[0]->id()), theFeatures[0]);
				for (unsigned int i=0; i<theFeatures.size(); ++i)
				{
					if (!theFeatures[i]->isDirty() && !theFeatures[i]->hasOSMId() && theFeatures[i]->isUploadable()) {
						bool userAdded = !theFeatures[i]->id().startsWith("conflict_");
						L->add(new AddFeatureCommand(Main->document()->getDirtyLayer(),theFeatures[i],userAdded));
					}
					L->add(new SetTagCommand(theFeatures[i],value.toString(),"", Main->document()->getDirtyOrOriginLayer(theFeatures[i]->layer())));
					theFeatures[i]->setLastUpdated(MapFeature::User);
				}
				Tags.push_back(std::make_pair(value.toString(),""));
				Main->document()->addHistory(L);
				endInsertRows();
			}
			else
				return false;
		}
		else
		{
			QString Original(Tags[index.row()].first);
			if (index.column() == 0)
				Tags[index.row()].first = value.toString();
			else
				Tags[index.row()].second = value.toString();
			CommandList* L = new CommandList(MainWindow::tr("Set Tags on %1").arg(theFeatures[0]->id()), theFeatures[0]);
			for (unsigned int i=0; i<theFeatures.size(); ++i)
			{
				unsigned int j = theFeatures[i]->findKey(Original);
				if (j<theFeatures[i]->tagSize()) {
					if (!theFeatures[i]->isDirty() && !theFeatures[i]->hasOSMId() && theFeatures[i]->isUploadable()) {
						bool userAdded = !theFeatures[i]->id().startsWith("conflict_");
						L->add(new AddFeatureCommand(Main->document()->getDirtyLayer(),theFeatures[i],userAdded));
					}
					L->add(new SetTagCommand(theFeatures[i],j , Tags[index.row()].first, Tags[index.row()].second, Main->document()->getDirtyOrOriginLayer(theFeatures[i]->layer())));
				}
				theFeatures[i]->setLastUpdated(MapFeature::User);
			}
			Main->document()->addHistory(L);
			Main->invalidateView(false);
		}
		emit dataChanged(index, index);
		return true;
	}
	return false;
}

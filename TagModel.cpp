#include "TagModel.h"
#include "MainWindow.h"
#include "Command/FeatureCommands.h"
#include "Map/MapDocument.h"
#include "Map/MapFeature.h"

TagModel::TagModel(MainWindow* aMain)
: Main(aMain)
{
}

TagModel::~TagModel(void)
{
}

void TagModel::setFeature(const std::vector<MapFeature*> Features)
{
	if (Tags.size())
	{
		beginRemoveRows(QModelIndex(),0,Tags.size());
		endRemoveRows();
	}
	theFeatures = Features;
	Tags.clear();
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
				Tags.push_back(std::make_pair(F->tagKey(i),F->tagValue(i)));
		}
		beginInsertRows(QModelIndex(),0,Tags.size()+1);
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
	if (index.row() > Tags.size())
		return QVariant();
	if (role == Qt::DisplayRole)
	{
		if (index.row() == Tags.size())
		{
			if (index.column() == 0)
				return "Edit this to add...";
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
	return QVariant();
}

QVariant TagModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();
	if (orientation == Qt::Horizontal)
	{
		if (section == 0)
			return "Key";
		else
			return "Value";
	}
	return QVariant();
}

Qt::ItemFlags TagModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled;
	return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

bool TagModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!theFeatures.size()) return false;
	if (index.isValid() && role == Qt::EditRole)
	{
		if (index.row() == Tags.size())
		{
			if (index.column() == 0)
			{
				beginInsertRows(QModelIndex(), Tags.size()+1, Tags.size()+2);
				CommandList* L = new CommandList;
				for (unsigned int i=0; i<theFeatures.size(); ++i)
				{
					L->add(new SetTagCommand(theFeatures[i],value.toString(),""));
					theFeatures[i]->setLastUpdated(MapFeature::User);
				}
				Tags.push_back(std::make_pair(value.toString(),""));
				Main->document()->history().add(L);
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
			CommandList* L = new CommandList;	
			for (unsigned int i=0; i<theFeatures.size(); ++i)
			{
				unsigned int j = theFeatures[i]->findKey(Original);
				if (j<theFeatures[i]->tagSize())
					L->add(new SetTagCommand(theFeatures[i],j , Tags[index.row()].first, Tags[index.row()].second));
				theFeatures[i]->setLastUpdated(MapFeature::User);
			}
			Main->document()->history().add(L);
			Main->invalidateView();
		}
		emit dataChanged(index, index);
		return true;
	}
	return false;
}






#ifndef MERKAARTOR_TAGMODEL_H_
#define MERKAARTOR_TAGMODEL_H_

#include <QtCore/QAbstractTableModel>
#include <QtCore/QString>

#include <utility>
#include <QList>
#include <QPair>

class MainWindow;
class Feature;

class TagModel : public QAbstractTableModel
{
Q_OBJECT
	public:
		TagModel(MainWindow* aMain);
		~TagModel();

		inline static const QString newKeyText(void)
		{ return tr("Edit this to add..."); }

		void setFeature(const QList<Feature*> Features);
		int rowCount(const QModelIndex &parent = QModelIndex()) const;
		int columnCount(const QModelIndex &parent = QModelIndex()) const;
		QVariant data(const QModelIndex &index, int role) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
		Qt::ItemFlags flags(const QModelIndex &index) const;
		bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
	private:
		MainWindow* Main;
		QList<Feature*> theFeatures;
		QList<QPair<QString, QString> > Tags;
};

#endif



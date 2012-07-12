#ifndef ACTIONSDIALOG_H
#define ACTIONSDIALOG_H

#include <qdialog.h>
#include <qlist.h>

class QAction;
class QTableWidget;
class QTableItem;
class QWidget;

class ActionsDialog : public QDialog
{
    Q_OBJECT

public:
    ActionsDialog(QList<QAction *>& actions);

protected slots:
    void accept();
    void resetToDefault();

private slots:
    void recordAction(int row, int column);
    void validateAction(int row, int column);
    void importShortcuts();
    void exportShortcuts();

private:
    QString oldAccelText;
    QTableWidget *actionsTable;
    QList<QAction*> actionsList;
};

#endif

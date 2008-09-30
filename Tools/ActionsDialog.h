#ifndef ACTIONSDIALOG_H
#define ACTIONSDIALOG_H

#include <qdialog.h>
#include <qlist.h>

class QAction;
class QTableWidget;
class QTableItem;
class QWidget;
class MainWindow;

class ActionsDialog : public QDialog
{
    Q_OBJECT

public:
    ActionsDialog(QList<QAction *>& actions, MainWindow *parent = 0);

protected slots:
    void accept();
    void resetToDefault();

private slots:
    void recordAction(int row, int column);
    void validateAction(int row, int column);

private:
	MainWindow* Main;
    QString oldAccelText;
    QTableWidget *actionsTable;
    QList<QAction*> actionsList;
};

#endif

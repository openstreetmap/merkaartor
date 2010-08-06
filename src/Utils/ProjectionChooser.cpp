#include "ProjectionChooser.h"
#include "ui_ProjectionChooser.h"

#include "MerkaartorPreferences.h"

ProjectionChooser::ProjectionChooser(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProjectionChooser)
{
    ui->setupUi(this);
}

ProjectionChooser::~ProjectionChooser()
{
    delete ui;
}

QString ProjectionChooser::getProjection(QString title, QWidget* parent)
{
    QString sPrj;

    ProjectionChooser* dlg = new ProjectionChooser(parent);
    dlg->setWindowTitle(title);

    foreach (ProjectionItem it, *M_PREFS->getProjectionsList()->getProjections()) {
        if (it.deleted)
            continue;
        dlg->ui->cbPredefined->addItem(it.name, it.projection);
    }

    dlg->ui->chkPredefined->setChecked(true);
    if (dlg->exec() == QDialog::Accepted) {
        if (dlg->ui->chkPredefined->isChecked())
            sPrj = dlg->ui->cbPredefined->itemText(dlg->ui->cbPredefined->currentIndex());
        else if (dlg->ui->chkStandard->isChecked())
            sPrj = dlg-> ui->txtStandard->text();
        else
            sPrj = dlg->ui->txtCustom->text();
    }

    delete dlg;
    return sPrj;
}

void ProjectionChooser::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

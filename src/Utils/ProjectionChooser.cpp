#include "ProjectionChooser.h"
#include "ui_ProjectionChooser.h"

#ifndef NO_PREFS
#include "MerkaartorPreferences.h"
#endif

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

#ifndef NO_PREFS
    int idx = 0, curIdx = 0;
    foreach (ProjectionItem it, *M_PREFS->getProjectionsList()->getProjections()) {
        if (it.deleted)
            continue;
        dlg->ui->cbPredefined->addItem(it.name, it.projection);
        if (it.name.contains(":4326"))
            curIdx = idx;
        ++idx;
    }
    dlg->ui->cbPredefined->setCurrentIndex(curIdx);
    dlg->ui->chkPredefined->setChecked(true);
#else
    dlg->ui->chkPredefined->setVisible(false);
    dlg->ui->cbPredefined->setVisible(false);
#endif
    dlg->adjustSize();

    if (dlg->exec() == QDialog::Accepted) {
        if (dlg->ui->chkPredefined->isChecked())
            sPrj = dlg->ui->cbPredefined->itemText(dlg->ui->cbPredefined->currentIndex());
        else if (dlg->ui->chkStandard->isChecked()) {
            sPrj = dlg-> ui->txtStandard->text().trimmed();
            bool ok;
            sPrj.toInt(&ok);
            if (ok)
                sPrj = "EPSG:" + sPrj;
        } else
            sPrj = dlg->ui->txtCustom->text().trimmed();
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

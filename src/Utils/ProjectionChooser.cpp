#include "ProjectionChooser.h"
#include "ui_ProjectionChooser.h"

#ifndef NO_PREFS
#include "MerkaartorPreferences.h"
#endif

#include "ogrsf_frmts.h"

#include <QMessageBox>

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

QString ProjectionChooser::getProjection(QString title, bool bShowPredefined, QString initialProj, QWidget* parent)
{
    QString sPrj;

    ProjectionChooser* dlg = new ProjectionChooser(parent);
    dlg->setWindowTitle(title);

#ifndef NO_PREFS
    if (bShowPredefined) {
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
    } else {
        dlg->ui->chkPredefined->setVisible(false);
        dlg->ui->cbPredefined->setVisible(false);
    }
#else
    dlg->ui->chkPredefined->setVisible(false);
    dlg->ui->cbPredefined->setVisible(false);
#endif

    if (!initialProj.isEmpty()) {
        if (initialProj.startsWith("+proj")) {
            dlg->ui->txtCustom->setText(initialProj);
            dlg->ui->chkCustom->setChecked(true);
        } else if (initialProj.startsWith("PROJCS")) {
            dlg->ui->txWkt->setPlainText(initialProj);
            dlg->ui->chkWkt->setChecked(true);
        }
    }

    dlg->adjustSize();

    if (dlg->exec() == QDialog::Accepted) {
        if (dlg->ui->chkPredefined->isChecked())
            sPrj = dlg->ui->cbPredefined->itemData(dlg->ui->cbPredefined->currentIndex()).toString();
        else if (dlg->ui->chkStandard->isChecked()) {
            sPrj = dlg-> ui->txtStandard->text().trimmed();
            bool ok;
            int iEpsg = sPrj.toInt(&ok);
            if (ok) {
                sPrj = "EPSG:" + sPrj;

                OGRSpatialReference *poSRS;
                poSRS = new OGRSpatialReference();
                poSRS->importFromEPSG(iEpsg);

                char* cTheProj4;
                if (poSRS->exportToProj4(&cTheProj4) != OGRERR_NONE) {
                    QMessageBox::critical(parent, tr("Error in WKT string"), tr("Cannot export to PROJ4 format"));
                    sPrj = QString();
                } else {
                    sPrj = QString(cTheProj4);
                }
                poSRS->Release();
            }
        } else if (dlg->ui->chkWkt->isChecked()) {
            OGRSpatialReference *poSRS;
            poSRS = new OGRSpatialReference();
            QByteArray ba = dlg->ui->txWkt->toPlainText().toLatin1();
            char* pszInput = ba.data();
            char** ppszInput = &pszInput;
            if (poSRS->importFromWkt(ppszInput) != OGRERR_NONE) {
                if (poSRS->importFromESRI(ppszInput) != OGRERR_NONE) {
                    QMessageBox::critical(parent, tr("Error in WKT string"), tr("Invalid WKT string"));
                    poSRS->Release();
                    sPrj = QString();
                }
            }
            poSRS->morphFromESRI();
            char* cTheProj4;
            if (poSRS->exportToProj4(&cTheProj4) != OGRERR_NONE) {
                QMessageBox::critical(parent, tr("Error in WKT string"), tr("Cannot export to PROJ4 format"));
                sPrj = QString();
            } else {
                sPrj = QString(cTheProj4);
            }
            poSRS->Release();
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

//***************************************************************
// CLass: %CLASS%
//
// Description:
//
//
// Author: Chris Browet <cbro@semperpax.com> (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//******************************************************************

#include "ImportCSVDialog.h"
#include "ui_ImportCSVDialog.h"
#include "Features.h"
#include "Layer.h"
#include "Global.h"

#include <QTimer>
#include <QMessageBox>
#include <QDomDocument>
#include <QFileDialog>

ImportCSVDialog::ImportCSVDialog(QIODevice* aDev, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImportCSVDialog)
    , m_dev(aDev)
    , m_selField(-1)
{
    ui->setupUi(this);
    ui->edCustomDelim->setEnabled(false);
    ui->cbFieldType->addItem(tr("String"), CSVString);
    ui->cbFieldType->addItem(tr("Integer"), CSVInt);
    ui->cbFieldType->addItem(tr("Float"), CSVFloat);
    ui->cbFieldType->addItem(tr("Longitude"), CSVLongitude);
    ui->cbFieldType->addItem(tr("Latitude"), CSVLatitude);

    CSVProjection.setProjectionType("EPSG:4326");

    QTimer::singleShot(0, this, SLOT(initialize()));
}

ImportCSVDialog::~ImportCSVDialog()
{
    delete ui;
}

void ImportCSVDialog::initialize()
{
    m_dev->seek(0);
    int l=0;
    QString previewText;
    while (l<4 && !m_dev->atEnd()) {
        QString line = m_dev->readLine();
        previewText += line;
        ++l;
    }
    ui->txInput->setText(previewText);

    analyze();
    generatePreview();
}

void ImportCSVDialog::analyze()
{
    m_dev->seek(0);
    QString l;
    QStringList hdr;
    bool ok;

    l = m_dev->readLine();

    QStringList slc = l.split(",");
    int best = slc.size();
    ui->rbCommaDelim->setChecked(true);
    m_delim = ",";

    QStringList sls = l.split(";");
    if (sls.size() > best) {
        best = sls.size();
        ui->rbSemiDelim->setChecked(true);
        m_delim = ";";
    }

    QStringList slt = l.split("\t");
    if (slt.size() > best) {
        best = slt.size();
        ui->rbTabDelim->setChecked(true);
        m_delim = "\t";
    }

    ui->cbHasHeader->setChecked(true);
    QStringList sl = l.split(m_delim);
    for (int i=0; i<sl.size(); ++i) {
        sl[i].toDouble(&ok);
        if (ok) {
            ui->cbHasHeader->setChecked(false);
            break;
        }
    }

    QRegularExpression rx(QString("%1\\s*\".*\"\\s*%1").arg(m_delim));
    if (l.indexOf(rx)) {
        m_quote = "\"";
        ui->rbStringDouble->setChecked(true);
    } else {
        rx = QRegularExpression(QString("%1\\s*'.*'\\s*%1").arg(m_delim));
        if (l.indexOf(rx)) {
            m_quote = "'";
            ui->rbStringSingle->setChecked(true);
        } else {
            m_quote.clear();
            ui->rbStringNone->setChecked(true);
        }

    }
    m_dev->seek(0);
    if (ui->cbHasHeader->isChecked()) {
        l = m_dev->readLine();
        hdr = l.split(m_delim);
    }
    l = m_dev->readLine();
    QStringList flds = l.split(m_delim);

    Fields.clear();
    int hdrIdx = 0;
    for (int i=0; i<flds.size(); ++i) {
        CSVField f;
        QString fld = flds[i];
        if (!m_quote.isEmpty()) {
            if (flds[i].trimmed().startsWith(m_quote)) {
                while (i<flds.size()-1 && !flds[i].trimmed().endsWith(m_quote)) {
                    ++i;
                    fld += m_delim + flds[i];
                }
                fld.remove(0, 1);
                fld.chop(1);
            }
        }
        if (ui->cbHasHeader->isChecked()) {
            if (hdrIdx<hdr.size())
                f.name = hdr[hdrIdx].simplified();
        } else
            f.name = "field_" + QString::number(i);
        flds[i].toDouble(&ok);
        if (ok) {
            if (f.name.startsWith("lat", Qt::CaseInsensitive)) {
                f.type = CSVLatitude;
            } else if (f.name.startsWith("lon", Qt::CaseInsensitive)) {
                f.type = CSVLongitude;
            } else
                f.type = CSVFloat;
        } else {
            flds[i].toInt(&ok);
            if (ok)
                f.type = CSVInt;
            else
                f.type = CSVString;
        }
        f.import = true;
        Fields << f;
        hdrIdx++;
    }

    ui->lvFields->clear();
    for (int i=0; i< Fields.size(); ++i) {
        QListWidgetItem* item = new QListWidgetItem(Fields[i].name);
        item->setData(Qt::UserRole, i);
        ui->lvFields->addItem(item);
    }

}

Feature* ImportCSVDialog::generateOSM(Layer* l, QString line)
{
    bool ok;
    QPointF p;
    qreal t;
    bool hasLat = false, hasLon = false;

    QStringList flds = line.split(m_delim);
    if (flds.size() < 2)
        return NULL;

    Node *N = g_backend.allocNode(l, Coord(0, 0));
    int lidx=0;
    for (int i=0; i<Fields.size(); ++i) {
        CSVField f = Fields[i];

        switch (f.type) {
        case CSVLatitude:
            if (f.import) {
                t = flds[lidx].toDouble(&ok);
                if (ok) {
                    p.setY(t);
                    hasLat = true;
                }
            }
            break;
        case CSVLongitude:
            if (f.import) {
                t = flds[lidx].toDouble(&ok);
                if (ok) {
                    p.setX(t);
                    hasLon = true;
                }
            }
            break;
        case CSVString: {
            QString fld = flds[lidx];
            if (!m_quote.isEmpty()) {
                if (flds[lidx].trimmed().startsWith(m_quote)) {
                    while (lidx<flds.size()-1 && !flds[lidx].trimmed().endsWith(m_quote)) {
                        ++lidx;
                        fld += m_delim + flds[lidx];
                    }
                    fld.remove(0, 1);
                    fld.chop(1);
                }
            }
            if (f.import)
                N->setTag(f.name, fld);
            break;
        }

        default:
            if (f.import)
                N->setTag(f.name, flds[lidx].trimmed());
            break;
        }
        ++lidx;
    }
    if (!hasLat || !hasLon) {
        g_backend.deallocFeature(l, N);
        return NULL;
    }
    if (CSVProjection.projIsLatLong())
        N->setPosition(p);
    else
        N->setPosition(CSVProjection.inverse(p));
    return N;
}

void ImportCSVDialog::generatePreview(int /*sel*/)
{
    m_dev->seek(0);
    QString line;
    QString previewText;

    if (ui->cbHasHeader)
        line = m_dev->readLine();

    int l=0;
    while (l<4 && !m_dev->atEnd()) {
        line = m_dev->readLine().trimmed();
        Feature* F = generateOSM(NULL, line);
        if (F) {
            previewText += F->toXML(2);
            delete F;
        }
        ++l;
    }
    ui->txPreview->setText(previewText);
}

void ImportCSVDialog::changeEvent(QEvent *e)
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

void ImportCSVDialog::on_lvFields_itemSelectionChanged()
{
    QListWidgetItem* it = ui->lvFields->item(ui->lvFields->currentRow());
    m_selField = -1;

    int idx = it->data(Qt::UserRole).toInt();
    if (idx >= Fields.size()) {
        return;
    }

    CSVField f(Fields[idx]);
    ui->edFieldName->setText(f.name);
    ui->cbFieldType->setCurrentIndex(f.type);
    ui->cbFieldImport->setChecked(f.import);

    m_selField = idx;
}

void ImportCSVDialog::on_rbCommaDelim_clicked()
{
    m_delim = ",";
    generatePreview();
}

void ImportCSVDialog::on_rbSemiDelim_clicked()
{
    m_delim = ";";
    generatePreview();
}

void ImportCSVDialog::on_rbTabDelim_clicked()
{
    m_delim = "\t";
    generatePreview();
}

void ImportCSVDialog::on_edCustomDelim_textEdited()
{
    m_delim = ui->edCustomDelim->text();
    generatePreview();
}

void ImportCSVDialog::on_rbStringNone_clicked()
{
    m_quote.clear();
    generatePreview();
}

void ImportCSVDialog::on_rbStringSingle_clicked()
{
    m_quote = "'";
    generatePreview();
}

void ImportCSVDialog::on_rbStringDouble_clicked()
{
    m_quote = "\"";
    generatePreview();
}

void ImportCSVDialog::on_edFieldName_textEdited()
{
    if (m_selField == -1)
        return;

    Fields[m_selField].name = ui->edFieldName->text();

    QListWidgetItem* it = ui->lvFields->item(ui->lvFields->currentRow());
    it->setText(ui->edFieldName->text());

    generatePreview();
}

void ImportCSVDialog::on_cbFieldType_currentIndexChanged(int index)
{
    if (m_selField == -1)
        return;

    Fields[m_selField].type = (CSVFieldType)index;

    generatePreview();
}

void ImportCSVDialog::on_cbFieldImport_clicked(bool b)
{
    if (m_selField == -1)
        return;

    Fields[m_selField].import = b;

    generatePreview();
}

void ImportCSVDialog::on_edProjection_editingFinished()
{
    if (ui->edProjection->text().isEmpty())
        CSVProjection.setProjectionType("EPSG:4326");
    else if (!CSVProjection.setProjectionType(ui->edProjection->text())) {
        QMessageBox::critical(0, tr("Invalid projection"), tr("Unable to set projection."));
        CSVProjection.setProjectionType("EPSG:4326");
    }

    generatePreview();
}

void ImportCSVDialog::on_buttonBox_accepted()
{
    bool hasLat = false;
    bool hasLon = false;
    foreach (CSVField f, Fields) {
        if (f.type == CSVLatitude)
            hasLat = true;
        if (f.type == CSVLongitude)
            hasLon = true;
    }

    if (!hasLat || !hasLon) {
        if (QMessageBox::critical(this, tr("No coordinates"),
                                  tr("Latitude or Longitude field missing. It will be impossible to import the file.\nDo you really want to exit?")
            , QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
            return;
        else
            reject();
    }

    accept();
}

bool ImportCSVDialog::import(Layer *aLayer)
{
    QString line;

    m_dev->seek(0);
    if (ui->cbHasHeader->isChecked())
        m_dev->readLine();

    int l = 0;
    while (l < ui->sbFrom->value() && !m_dev->atEnd()) {
        m_dev->readLine();
        ++l;
    }

    while ((l < ui->sbTo->value() || ui->sbTo->value() == 0) && !m_dev->atEnd()) {
        line = m_dev->readLine().trimmed();
        Feature* F = generateOSM(aLayer, line);
        if (F)
            aLayer->add(F);

        ++l;
    }
    return true;
}

void ImportCSVDialog::on_btLoad_clicked()
{
    QString f = QFileDialog::getOpenFileName(this, tr("Load CSV import settings"), QString(), tr("Merkaartor import settings (*.mis)"));
    if (f.isEmpty())
        return;

    QFile file(f);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Invalid file"), tr("%1 could not be opened.").arg(f));
        return;
    }

    QDomDocument theXmlDoc;
    if (!theXmlDoc.setContent(&file)) {
        file.close();
        return;
    }
    file.close();

    QDomElement docElem = theXmlDoc.documentElement();
    if (docElem.tagName() != "CSVImportSettings") {
        QMessageBox::critical(this, tr("Invalid file"), tr("%1 is not a CSV import settings file").arg(f));
        return;
    }

    m_delim = docElem.attribute("delimiter");
    if (m_delim == ",")
        ui->rbCommaDelim->setChecked(true);
    else if (m_delim == ";")
        ui->rbSemiDelim->setChecked(true);
    else if (m_delim == "tab") {
        m_delim = "\t";
        ui->rbTabDelim->setChecked(true);
    } else {
        ui->rbCustomDelim->setChecked(true);
        ui->edCustomDelim->setText(m_delim);
    }

    ui->cbHasHeader->setChecked(docElem.attribute("header") == "true" ? true : false);
    ui->sbFrom->setValue(docElem.attribute("from").toInt());
    ui->sbTo->setValue(docElem.attribute("to").toInt());

    Fields.clear();
    QDomElement e = docElem.firstChildElement();
    while(!e.isNull()) {
        if (e.tagName() == "Fields") {
            QDomElement c = e.firstChildElement();
            while(!c.isNull()) {
                if (c.tagName() == "Field") {
                    CSVField f;
                    f.name = c.attribute("name");
                    f.type = (CSVFieldType)c.attribute("type").toInt();
                    f.import = c.attribute("import") == "false" ? false : true;

                    Fields << f;
                }

                c = c.nextSiblingElement();
            }
        }
        else if (e.tagName() == "Projection") {
            ui->edProjection->setText(e.text().trimmed());
        }
        e = e.nextSiblingElement();
    }

    ui->lvFields->clear();
    for (int i=0; i< Fields.size(); ++i) {
        QListWidgetItem* item = new QListWidgetItem(Fields[i].name);
        item->setData(Qt::UserRole, i);
        ui->lvFields->addItem(item);
    }
    ui->edFieldName->setText(QString());

    on_edProjection_editingFinished();
}

void ImportCSVDialog::on_btSave_clicked()
{
    QDomDocument theXmlDoc;

    theXmlDoc.appendChild(theXmlDoc.createProcessingInstruction("xml", "version=\"1.0\""));

    QDomElement root = theXmlDoc.createElement("CSVImportSettings");
    theXmlDoc.appendChild(root);
    root.setAttribute("creator", QString("%1 v%2%3").arg(qApp->applicationName()).arg(BuildMetadata::VERSION).arg(BuildMetadata::REVISION));

    QString tDelim = m_delim;
    if (tDelim == "\t")
        tDelim = "tab";
    root.setAttribute("delimiter", tDelim);
    root.setAttribute("header", ui->cbHasHeader->isChecked() ? "true" : "false");
    root.setAttribute("from", QString::number(ui->sbFrom->value()));
    root.setAttribute("to", QString::number(ui->sbTo->value()));

    QDomElement p = theXmlDoc.createElement("Projection");
    root.appendChild(p);
    QDomText t = theXmlDoc.createTextNode(ui->edProjection->text());
    p.appendChild(t);

    QDomElement flds = theXmlDoc.createElement("Fields");
    root.appendChild(flds);

    foreach(CSVField f, Fields) {
        QDomElement fld = theXmlDoc.createElement("Field");
        flds.appendChild(fld);

        fld.setAttribute("name", f.name);
        fld.setAttribute("type", QString::number(f.type));
        fld.setAttribute("import", f.import ? "true" : "false");
    }

    QString f;
    QFileDialog dlg(this, tr("Save CSV import settings"), QString("%1/%2.mis").arg(M_PREFS->getworkingdir()).arg(tr("untitled")), tr("Merkaartor import settings (*.mis)") + "\n" + tr("All Files (*)"));
    dlg.setFileMode(QFileDialog::AnyFile);
    dlg.setDefaultSuffix("mis");
    dlg.setAcceptMode(QFileDialog::AcceptSave);

    if (dlg.exec()) {
        if (dlg.selectedFiles().size())
            f = dlg.selectedFiles()[0];
    }
//    f = QFileDialog::getSaveFileName(this, tr("Save CSV import settings"), "", tr("Merkaartor import settings (*.mis)"));

    if (f.isEmpty())
        return;

    QFile file(f);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
       QMessageBox::critical(this, tr("Unable to open save import settings"), tr("%1 could not be opened for writing.").arg(f));
        return;
    }
    file.write(theXmlDoc.toString().toUtf8());
    file.close();
}

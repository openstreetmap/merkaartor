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
    ui->rbColonDelim->setChecked(true);
    delim = ",";

    QStringList sls = l.split(";");
    if (sls.size() > best) {
        best = sls.size();
        ui->rbSemiDelim->setChecked(true);
        delim = ";";
    }

    QStringList slt = l.split("\t");
    if (slt.size() > best) {
        best = slt.size();
        ui->rbTabDelim->setChecked(true);
        delim = "\t";
    }

    ui->cbHasHeader->setChecked(true);
    QStringList sl = l.split(delim);
    for (int i=0; i<sl.size(); ++i) {
        qreal r = sl[i].toDouble(&ok);
        if (ok) {
            ui->cbHasHeader->setChecked(false);
            break;
        }
    }

    m_dev->seek(0);
    if (ui->cbHasHeader->isChecked()) {
        l = m_dev->readLine();
        hdr = l.split(delim);
    }
    l = m_dev->readLine();
    QStringList fld = l.split(delim);

    Fields.clear();
    for (int i=0; i<fld.size(); ++i) {
        CSVField f;
        if (ui->cbHasHeader->isChecked())
            f.name = hdr[i].simplified();
        else
            f.name = "field_" + QString::number(i);
        int t1 = fld[i].toInt(&ok);
        if (ok)
            f.type = CSVInt;
        else {
            double t2 = fld[i].toDouble(&ok);
            if (ok)
                f.type = CSVFloat;
            else
                f.type = CSVString;
        }
        f.import = true;
        Fields << f;
    }

    ui->lvFields->clear();
    for (int i=0; i< Fields.size(); ++i) {
        QListWidgetItem* item = new QListWidgetItem(Fields[i].name);
        item->setData(Qt::UserRole, i);
        ui->lvFields->addItem(item);
    }

}

Feature* ImportCSVDialog::generateOSM(QString line)
{
    bool ok;
    QPointF p;
    double t;
    bool hasLat = false, hasLon = false;

    QStringList sl = line.split(delim);
    if (sl.size() < 2)
        return NULL;

    Node *N = new Node(Coord(0, 0));
    for (int i=0; i<Fields.size(); ++i) {
        CSVField f = Fields[i];
        if (!f.import)
            continue;

        switch (f.type) {
        case CSVLatitude:
            t = sl[i].toDouble(&ok);
            if (ok) {
                p.setY(t);
                hasLat = true;
            }
            break;
        case CSVLongitude:
            t = sl[i].toDouble(&ok);
            if (ok) {
                p.setX(t);
                hasLon = true;
            }
            break;
        default:
            N->setTag(f.name, sl[i].simplified());
            break;
        }
    }
    if (!hasLat || !hasLon) {
        delete N;
        return NULL;
    }
    N->setPosition(CSVProjection.inverse(p));
    return N;
}

void ImportCSVDialog::generatePreview(int sel)
{
    m_dev->seek(0);
    QString line;
    QString previewText;

    if (ui->cbHasHeader)
        line = m_dev->readLine();

    int l=0;
    while (l<4 && !m_dev->atEnd()) {
        line = m_dev->readLine();
        Feature* F = generateOSM(line);
        if (F) {
            previewText += F->toXML(0, NULL);
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

void ImportCSVDialog::on_rbColonDelim_clicked()
{
    delim = ",";

    generatePreview();
}

void ImportCSVDialog::on_rbSemiDelim_clicked()
{
    delim = ";";

    generatePreview();
}

void ImportCSVDialog::on_rbTabDelim_clicked()
{
    delim = "\t";

    generatePreview();
}

void ImportCSVDialog::on_edCustomDelim_textEdited()
{
    delim = ui->edCustomDelim->text();

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
    if (ui->cbHasHeader)
        m_dev->readLine();

    int l = 0;
    while (l < ui->sbFrom->value() && !m_dev->atEnd()) {
        m_dev->readLine();
        ++l;
    }

    while ((l < ui->sbTo->value() || ui->sbTo == 0) && !m_dev->atEnd()) {
        line = m_dev->readLine();
        Feature* F = generateOSM(line);
        if (F)
            aLayer->add(F);

        ++l;
    }
    return true;
}

void ImportCSVDialog::on_btLoad_clicked()
{
    QString f = QFileDialog::getOpenFileName(this, tr("Load CSV import settings"), "", tr("Merkaartor import settings (*.mis)"));
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

    delim = docElem.attribute("delimiter");
    if (delim == ",")
        ui->rbColonDelim->setChecked(true);
    else if (delim == ";")
        ui->rbSemiDelim->setChecked(true);
    else if (delim == "tab") {
        delim = "\t";
        ui->rbTabDelim->setChecked(true);
    } else {
        ui->rbCustomDelim->setChecked(true);
        ui->edCustomDelim->setText(delim);
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
    ui->edFieldName->setText("");

    on_edProjection_editingFinished();
}

void ImportCSVDialog::on_btSave_clicked()
{
    QDomDocument theXmlDoc;

    theXmlDoc.appendChild(theXmlDoc.createProcessingInstruction("xml", "version=\"1.0\""));

    QDomElement root = theXmlDoc.createElement("CSVImportSettings");
    theXmlDoc.appendChild(root);
    root.setAttribute("creator", QString("Merkaartor v%1%2").arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)));

    QString tDelim = delim;
    if (tDelim == "\t")
        tDelim = "tab";
    root.setAttribute("delimiter", tDelim);
    root.setAttribute("header", ui->cbHasHeader->isChecked() ? "true" : false);
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

    QString f = QFileDialog::getSaveFileName(this, tr("Save CSV import settings"), "", tr("Merkaartor import settings (*.mis)"));
    if (f.isEmpty())
        return;

    if (!f.endsWith(".mis"))
        f.append(".mis");

    QFile file(f);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
       QMessageBox::critical(this, tr("Unable to open save import settings"), tr("%1 could not be opened for writing.").arg(f));
        return;
    }
    file.write(theXmlDoc.toString().toUtf8());
    file.close();
}

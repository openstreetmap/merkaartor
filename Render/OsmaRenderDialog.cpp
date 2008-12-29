//
// C++ Implementation: OsmaRenderDialog
//
// Description:
//
//
// Author: Chris Browet <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "OsmaRenderDialog.h"

#include "Map/MapDocument.h"
#include "Utils/PictureViewerDialog.h"

#include <QtSvg>
#include <QFileDialog>
#include <QFileInfo>
#include <QProgressDialog>

#include <libxml/xmlmemory.h>
#include <libxml/debugXML.h>
#include <libxml/HTMLtree.h>
#include <libxml/xmlIO.h>
#include <libxml/xinclude.h>
#include <libxml/catalog.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libexslt/exslt.h>

#ifdef QGDAL_FEAT
	#include "qgdal.h"
#endif

void xsltCallback (void * /*ctx*/, const char * msg, ...)
{
    char buf[1024];
    va_list ap;
    va_start( ap, msg );
    vsnprintf( buf, 1024, msg, ap );
	qDebug("%s", buf);
    va_end( ap );

	QCoreApplication::processEvents();
}

OsmaRenderDialog::OsmaRenderDialog(MapDocument *aDoc, const CoordBox& aCoordBox, QWidget *parent)
	:QDialog(parent), theDoc(aDoc)
{
	setupUi(this);
	buttonBox->addButton("Proceed...", QDialogButtonBox::ActionRole);

	SvgSets = new QSettings();
	SvgSets->beginGroup("OsmaRenderDialog");

	svgOutputFilename->setText(SvgSets->value("svgOutputFilename", "").toString());
	sbZoom->setValue(SvgSets->value("sbZoom", "12").toInt());
	sbScale->setValue(SvgSets->value("sbScale", "1.0").toDouble());
	cbShowScale->setCheckState((Qt::CheckState)SvgSets->value("cbShowScale", "1").toInt());
	cbShowGrid->setCheckState((Qt::CheckState)SvgSets->value("cbShowGrid", "1").toInt());
	cbShowBorders->setCheckState((Qt::CheckState)SvgSets->value("cbShowBorders", "1").toInt());
	cbShowLicense->setCheckState((Qt::CheckState)SvgSets->value("cbShowLicense", "1").toInt());
	//cbInteractive->setChecked((Qt::CheckState)SvgSets->value("cbInteractive", "1").toInt());


	InkPath = MerkaartorPreferences::instance()->getTool("Inkscape").ToolPath;
	if (QFileInfo(InkPath).isExecutable()) {
		gbPreviewOK->setVisible(true);
		gbPreviewNOK->setVisible(false);
	} else {
		gbPreviewOK->setVisible(false);
		gbPreviewNOK->setVisible(true);
	}

	sbMinLat->setValue(intToAng(aCoordBox.bottomLeft().lat()));
	sbMaxLat->setValue(intToAng(aCoordBox.topLeft().lat()));
	sbMinLon->setValue(intToAng(aCoordBox.topLeft().lon()));
	sbMaxLon->setValue(intToAng(aCoordBox.topRight().lon()));

#ifdef QGDAL_FEAT
	QGDAL g;
//	if (g.load("/xxvar/src/merkaartor_trunk/Data/SP27GTIF.TIF")) {
	if (g.load("c:/home/cbrowet/src/merkaartor_trunk/Data/SP27GTIF.TIF")) {
		g.printInfo();
	}
#endif

	sbDPI->setValue(SvgSets->value("sbDPI", "90").toInt());
	if (SvgSets->value("cbShowPreview", "false").toBool()) {
		cbShowPreview->setChecked(true);
		sbDPI->setEnabled(true);
	}

	refreshLabels();
	resize(1,1);
}

void OsmaRenderDialog::on_btBrowseOutFilename_clicked()
{
	QString s = QFileDialog::getSaveFileName(this,tr("SVG output filename"),"",tr("SVG file (*.svg)"));
	if (!s.isNull()) {
		OutFilename = s;
		svgOutputFilename->setText(OutFilename);
	}
}

void OsmaRenderDialog::on_buttonBox_clicked(QAbstractButton * button)
{
	if (buttonBox->buttonRole(button) == QDialogButtonBox::ActionRole) {
		SvgSets->setValue("svgOutputFilename", svgOutputFilename->text());
		SvgSets->setValue("sbZoom", sbZoom->value());
		SvgSets->setValue("sbScale", sbScale->value());
		SvgSets->setValue("cbShowScale", cbShowScale->checkState());
		SvgSets->setValue("cbShowGrid", cbShowGrid->checkState());
		SvgSets->setValue("cbShowBorders", cbShowBorders->checkState());
		SvgSets->setValue("cbShowLicense", cbShowLicense->checkState());
		//SvgSets->setValue("cbInteractive", cbInteractive->checkState());

		SvgSets->setValue("cbShowPreview", cbShowPreview->isChecked());
		SvgSets->setValue("sbDPI", sbDPI->value());

		render();
	}
}

void OsmaRenderDialog::render()
{
	xsltStylesheetPtr cur = NULL;
	xmlDocPtr doc, xsl, res;

	if (svgOutputFilename->text().isEmpty()) {
		QMessageBox::warning(this,
			QApplication::tr("Invalid filename"),
			QApplication::tr("Please provide a valid output filename"));
		return;
	}

	CoordBox theCoordBox(Coord(angToInt(sbMinLat->value()), angToInt(sbMinLon->value())),
		Coord(angToInt(sbMaxLat->value()), angToInt(sbMaxLon->value())));
	QString theExport;
	QFile file(QDir::tempPath()+"/tmp.osm");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return;

	file.write(theDoc->exportOSM(theCoordBox, true).toUtf8());
	file.close();

	xmlSubstituteEntitiesDefault(1);
	xmlLoadExtDtdDefaultValue = 1;

	QString StyleSheetFileName = ":/osmarender/osmarender.xsl";
	QFile xslFile(StyleSheetFileName);
	if (!xslFile.open(QIODevice::ReadOnly)) {
		QMessageBox::critical(this,
			QApplication::tr("Unable to read stylesheet file"),
			QApplication::tr("Please make sure the Osmarender stylesheet is available at %1").arg(StyleSheetFileName));
		return;
	}
	QByteArray theXsl = xslFile.readAll();
	xslFile.close();

	xsl = xmlParseMemory(theXsl.data(), theXsl.size());
	if (!xsl)
	{
		QMessageBox::critical(this,
			QApplication::tr("Unable to parse stylesheet xml"),
			QApplication::tr("Please make sure the Osmarender stylesheet is available at %1").arg(StyleSheetFileName));
		return;
	}
	exsltCommonRegister();
	cur = xsltParseStylesheetDoc(xsl);
	if (!cur)
	{
		QMessageBox::critical(this,
			QApplication::tr("Unable to parse stylesheet"),
			QApplication::tr("Please make sure the Osmarender stylesheet is available at %1").arg(StyleSheetFileName));
		return;
	}

	QString FeatureFileName = QString(":/osmarender/osm-map-features-z%1.xml").arg(sbZoom->value());
	QFile xmlFile(FeatureFileName);
	if (!xmlFile.open(QIODevice::ReadOnly)) {
		QMessageBox::critical(this,
			QApplication::tr("Unable to read feature xml file"),
			QApplication::tr("Please make sure the feature xml is available at %1").arg(FeatureFileName));
		return;
	}
	QByteArray theDoc = xmlFile.readAll();
	xmlFile.close();

	doc = xmlParseMemory(theDoc.data(), theDoc.size());
	if (!doc)
	{
		QMessageBox::critical(this,
			QApplication::tr("Unable to parse feature xml"),
			QApplication::tr("Please make sure the feature xml is available at %1").arg(FeatureFileName));
		return;
	}


	const char * params[15];
	const xmlChar *string;
	xmlChar *value = NULL;
	xmlChar *valScale = NULL;

	QApplication::setOverrideCursor(Qt::WaitCursor);
	QProgressDialog progress("Generating the SVG. Please Wait...", "Cancel", 0, 0);
	progress.setWindowModality(Qt::WindowModal);
	progress.setCancelButton(NULL);
	progress.show();

	QApplication::processEvents();

	QByteArray ba(QFileInfo(file).absoluteFilePath().toLatin1());
	string = (const xmlChar *) ba.data();
	//string = (const xmlChar *) QFileInfo(file).absoluteFilePath().toAscii().data();
	value = xmlStrdup((const xmlChar *)"'");
	value = xmlStrcat(value, string);
	value = xmlStrcat(value, (const xmlChar *)"'");
	int i=-1;
	params[++i] = "osmfile";
	params[++i] = (const char *) value;
	params[++i] = "symbolsDir";
	params[++i] = (const char *) "xx";
	if (cbShowGrid->checkState() != Qt::PartiallyChecked) {
		params[++i] = "showGrid";
		params[++i] = (const char *) (cbShowGrid->isChecked() ? "'yes'" : "'no'");
	}
	if (cbShowBorders->checkState() != Qt::PartiallyChecked) {
		params[++i] = "showBorder";
		params[++i] = (const char *) (cbShowBorders->isChecked() ? "'yes'" : "'no'");
	}
	if (cbShowScale->checkState() != Qt::PartiallyChecked) {
		params[++i] = "showScale";
		params[++i] = (const char *) (cbShowScale->isChecked() ? "'yes'" : "'no'");
	}
	if (cbShowLicense->checkState() != Qt::PartiallyChecked) {
		params[++i] = "showLicense";
		params[++i] = (const char *) (cbShowLicense->isChecked() ? "'yes'" : "'no'");
	}
	if (sbScale->value() != 1.0) {
		params[++i] = "scale";
		valScale = xmlStrncatNew(valScale, (const xmlChar *)QString("%1").arg(QString::number(sbScale->value(),'f',1)).toUtf8().data(), -1);
		params[++i] = (const char *) valScale;
	}
	//params[++i] = "interactive";
	//params[++i] = (const char *) (cbInteractive->isChecked() ? "'yes'" : "'no'");
//	params[++i] = QString("'"+file.fileName()+"'").toLatin1().data();
//	params[++i] = "c:/home/cbrowet/Programming/merkaartor/Data/tmp.osm";
//	params[++i] = "c:\\home\\cbrowet\\Programming\\merkaartor\\Data\\tmp.osm";
	params[++i] = NULL;

	xsltSetGenericDebugFunc(NULL, xsltCallback);
	res = xsltApplyStylesheet(cur, doc, params);

	xmlChar* doc_txt_ptr;
	int doc_txt_len;
	xsltSaveResultToString(&doc_txt_ptr, &doc_txt_len, res, cur);
	QByteArray svg((const char *)doc_txt_ptr);

	QFile F(svgOutputFilename->text());
	F.open(QIODevice::WriteOnly | QIODevice::Truncate);
	F.write(svg);
	F.close();

	xsltFreeStylesheet(cur);
	xmlFreeDoc(res);
	xmlFreeDoc(doc);

	xsltCleanupGlobals();
	xmlCleanupParser();

	if (cbShowPreview->isChecked()) {
		QStringList InkParam;
		InkParam << "-z" <<  "-C" << "-f" <<  QDir::toNativeSeparators(svgOutputFilename->text())
			<< "-e" << QDir::toNativeSeparators(QDir::tempPath()+"/tmp.png")
			<< "-d" << QString::number(sbDPI->value());
			//<< "-w" <<  QString::number(sbPreviewWidth->value())
			//<< "-h" << QString::number(sbPreviewHeight->value());

		progress.setLabelText("Generating the PNG preview. Please Wait...");
		QApplication::processEvents();

		if (QProcess::execute(InkPath, InkParam) != 0)
			QMessageBox::warning(this,
				QApplication::tr("Unable to generate preview"),
				QApplication::tr("Preview generation failed. Please ensure Inkscape is properly installed. at %1").arg(QDir::toNativeSeparators(InkPath)));

		progress.reset();
		QApplication::restoreOverrideCursor();

		PictureViewerDialog vwDlg(svgOutputFilename->text(), QDir::tempPath()+"/tmp.png", this);
		vwDlg.exec();
	} else {
		progress.reset();
		QApplication::restoreOverrideCursor();
	}


//	QDialog dlg;
//	QSvgWidget* svgW = new QSvgWidget();
////	QSvgRenderer* svgR = new QSvgRenderer(svg);
////	svgW->load(ret);
//	svgW->load(svg);
////	svgW->load(QString("C:/home/cbrowet/Programming/merkaartor/tmp.svg"));
////	svgW->load(QString("/var/src/merkaartor/tst.svg"));
//	QSize s = svgW->renderer()->defaultSize();
//
//	QVBoxLayout* lay = new QVBoxLayout();
//
//	lay->addWidget(svgW);
//	dlg.setLayout(lay);
//
//	dlg.exec();

}

void OsmaRenderDialog::refreshLabels()
{
	double prj = (1/cos(angToRad((sbMaxLat->value() + sbMinLat->value())/2)));
/*	double km = (0.0089928 * sbScale->value() * 10000 * prj); */

	lblInfoSVG->setText(tr("The SVG will have a size of approx. %1 x %2 pixels (without extras like scale, borders, ...)")
		.arg(int((sbMaxLon->value() - sbMinLon->value()) * 10000 * sbScale->value()))
		.arg(int((sbMaxLat->value() - sbMinLat->value()) * 10000 * sbScale->value() * prj)));
	lbInfo->setText(tr("The bitmap will have a size of approx. %1 x %2 pixels (without extras like scale, borders, ...)\n")
		.arg(int((sbMaxLon->value() - sbMinLon->value()) * 10000 * sbScale->value() * (double(sbDPI->value() / 90.0))))
		.arg(int((sbMaxLat->value() - sbMinLat->value()) * 10000 * sbScale->value() * prj * (double(sbDPI->value() / 90.0))))
		+ tr("It will be saved as '%1'.").arg(svgOutputFilename->text()+".png"));
}

void OsmaRenderDialog::on_sbMinLat_valueChanged(double /* v */)
{
	refreshLabels();
}

void OsmaRenderDialog::on_sbMinLon_valueChanged(double /* v */)
{
	refreshLabels();
}

void OsmaRenderDialog::on_sbMaxLat_valueChanged(double /* v */)
{
	refreshLabels();
}

void OsmaRenderDialog::on_sbMaxLon_valueChanged(double /* v */)
{
	refreshLabels();
}

void OsmaRenderDialog::on_sbDPI_valueChanged(int /* v */)
{
	refreshLabels();
}

void OsmaRenderDialog::on_sbScale_valueChanged(double /* v */)
{
	refreshLabels();
}

void OsmaRenderDialog::on_svgOutputFilename_textChanged()
{
	refreshLabels();
}



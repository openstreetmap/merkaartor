#include "OsmaRender.h"

#include <QtGui/QMessageBox>
#include <QtSvg>
//#include <QValueVector>

#include "Map/MapDocument.h"
#include "Map/MapLayer.h"

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

OsmaRender::OsmaRender(void)
{
}

OsmaRender::~OsmaRender(void)
{
}

void OsmaRender::render(QWidget* aParent, MapDocument *aDoc, const CoordBox& aCoordBox)
{
	xsltStylesheetPtr cur = NULL;
	xmlDocPtr doc, res;

	QFile file(QDir::tempPath()+"/tmp.osm");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return;

	QTextStream out(&file);
	out << aDoc->exportOSM(aCoordBox);
	file.close();

	xmlSubstituteEntitiesDefault(1);
	xmlLoadExtDtdDefaultValue = 1;

#ifdef Q_OS_WIN32
	QString src_path("C:/home/cbrowet/Programming/merkaartor");
#else
	QString src_path("/var/src/merkaartor");
#endif
	QString StyleSheetFileName = src_path+"/osmarender6/osmarender.xsl";
	cur = xsltParseStylesheetFile((const xmlChar *)QDir(StyleSheetFileName).path().toLatin1().data());
	if (!cur)
	{
		QMessageBox::warning(aParent, 
			"Unable to read stylesheet",
			QString("Please make sure the Osmarender stylesheet is available at %1").arg(StyleSheetFileName));
		return;
	}

	doc = xmlParseFile((const char *)QDir(src_path+"/osmarender6/osm-map-features-z12.xml").path().toLatin1().data());

	const char * params[3];
	const xmlChar *string;
	xmlChar *value;

	QByteArray ba(QFileInfo(file).absoluteFilePath().toLatin1());
	string = (const xmlChar *) ba.data();
	//string = (const xmlChar *) QFileInfo(file).absoluteFilePath().toAscii().data();
	value = xmlStrdup((const xmlChar *)"'");
	value = xmlStrcat(value, string);
	value = xmlStrcat(value, (const xmlChar *)"'");
	params[0] = "osmfile";
	params[1] = (const char *) value;
//	params[1] = QString("'"+file.fileName()+"'").toLatin1().data();
//	params[1] = "c:/home/cbrowet/Programming/merkaartor/Data/tmp.osm";
//	params[1] = "c:\\home\\cbrowet\\Programming\\merkaartor\\Data\\tmp.osm";
	params[2] = NULL;
	res = xsltApplyStylesheet(cur, doc, params);

	xmlChar* doc_txt_ptr;
	int doc_txt_len;
	xsltSaveResultToString(&doc_txt_ptr, &doc_txt_len, res, cur);
	QByteArray svg((const char *)doc_txt_ptr);

	QFile F(QDir::tempPath()+"/tmp.svg");
	F.open(QIODevice::WriteOnly | QIODevice::Truncate);
	F.write(svg);
	F.close();

	xsltFreeStylesheet(cur);
	xmlFreeDoc(res);
	xmlFreeDoc(doc);

	xsltCleanupGlobals();
	xmlCleanupParser();

	QDialog dlg;
	QSvgWidget* svgW = new QSvgWidget();
//	QSvgRenderer* svgR = new QSvgRenderer(svg);
//	svgW->load(ret);
	svgW->load(svg);
//	svgW->load(QString("C:/home/cbrowet/Programming/merkaartor/tmp.svg"));
//	svgW->load(QString("/var/src/merkaartor/tst.svg"));
	QSize s = svgW->renderer()->defaultSize();

	QVBoxLayout* lay = new QVBoxLayout();

	lay->addWidget(svgW);
	dlg.setLayout(lay);

	dlg.exec();
}

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

#include "GdalAdapter.h"

#include <QCoreApplication>
#include <QtPlugin>
#include <QAction>
#include <QFileDialog>
#include <QPainter>
#include <QMessageBox>
#include <QInputDialog>

#include <QDebug>

#include "gdal_priv.h"
#include "ogrsf_frmts.h"

#include "ProjectionChooser.h"

#define IN_MEMORY_LIMIT 100000000

static const QUuid theUid ("{5c9479df-0b1a-4c49-9559-83d5ffa93911}");
static const QString theName("GDAL Raster");

QUuid GdalAdapterFactory::getId() const
{
    return theUid;
}

QString	GdalAdapterFactory::getName() const
{
    return theName;
}

/**************/

inline double radToAng(double a)
{
    return a*180/M_PI;
}

inline double angToRad(double a)
{
    return a*M_PI/180.;
}

#define FILTER_OPEN_SUPPORTED \
    tr("All Files (*)")

GdalAdapter::GdalAdapter()
    : poDataset(0), isLatLon(false)
{
    GDALAllRegister();

    QAction* loadImage = new QAction(tr("Load file(s)..."), this);
    loadImage->setData(theUid.toString());
    connect(loadImage, SIGNAL(triggered()), SLOT(onLoadImage()));
    QAction* setSource = new QAction(tr("Specify \"source\" tag..."), this);
    setSource->setData(theUid.toString());
    connect(setSource, SIGNAL(triggered()), SLOT(onSetSourceTag()));

    theMenu = new QMenu();
    theMenu->addAction(loadImage);
    theMenu->addAction(setSource);
}


GdalAdapter::~GdalAdapter()
{
    cleanup();
}

QUuid GdalAdapter::getId() const
{
    return theUid;
}

QString	GdalAdapter::getName() const
{
    return theName;
}

bool GdalAdapter::alreadyLoaded(QString fn) const
{
    for (int j=0; j<theImages.size(); ++j)
        if (theImages[j].theFilename == fn)
            return true;
    return false;
}

bool GdalAdapter::loadImage(const QString& fn)
{
    if (alreadyLoaded(fn))
        return true;

    QFileInfo fi(fn);
    GdalImage img;
    QRectF bbox;

    poDataset = (GDALDataset *) GDALOpen( QDir::toNativeSeparators(fi.absoluteFilePath()).toUtf8().constData(), GA_ReadOnly );
    if( poDataset == NULL )
    {
        qDebug() <<  "GDAL Open failed: " << fn;
        return false;
    }

    bool hasGeo = false;
    QDir dir(fi.absoluteDir());
    QString f = fi.baseName();
    QStringList wldFilter;
    wldFilter <<  f+".tfw" << f+".tifw" << f+".tiffw" << f+".wld";
    QFileInfoList fil = dir.entryInfoList(wldFilter);
    if (fil.count()) {
        QFile wld(fil[0].absoluteFilePath());
        if (wld.open(QIODevice::ReadOnly)) {
            int i;
            for (i=0; i<6; ++i) {
                if (wld.atEnd())
                    break;
                QString l = wld.readLine();
                bool ok;
                double d = l.toDouble(&ok);
                if (!ok)
                    break;
                switch (i) {
                case 0:
                    img.adfGeoTransform[1] = d;
                    break;
                case 1:
                    img.adfGeoTransform[4] = d;
                    break;
                case 2:
                    img.adfGeoTransform[2] = d;
                    break;
                case 3:
                    img.adfGeoTransform[5] = d;
                    break;
                case 4:
                    img.adfGeoTransform[0] = d;
                    break;
                case 5:
                    img.adfGeoTransform[3] = d;
                    break;
                }

            }
            if (i == 6)
                hasGeo = true;
        }
    }
    if(!hasGeo)
        if ( poDataset->GetGeoTransform( img.adfGeoTransform ) != CE_None ) {
            GDALClose((GDALDatasetH)poDataset);
            return false;
        }

    qDebug( "Origin = (%.6f,%.6f)\n",
            img.adfGeoTransform[0], img.adfGeoTransform[3] );

    qDebug( "Pixel Size = (%.6f,%.6f)\n",
            img.adfGeoTransform[1], img.adfGeoTransform[5] );

    bbox.setTopLeft(QPointF(img.adfGeoTransform[0], img.adfGeoTransform[3]));
    bbox.setWidth(img.adfGeoTransform[1]*poDataset->GetRasterXSize());
    bbox.setHeight(img.adfGeoTransform[5]*poDataset->GetRasterYSize());

    isLatLon = false;
    if( strlen(poDataset->GetProjectionRef()) != 0 ) {
        qDebug( "Projection is `%s'\n", poDataset->GetProjectionRef() );
        OGRSpatialReference* theSrs = new OGRSpatialReference(poDataset->GetProjectionRef());
        if (theSrs && theSrs->Validate() == OGRERR_NONE) {
            theSrs->morphFromESRI();
            char* theProj4;
            if (theSrs->exportToProj4(&theProj4) == OGRERR_NONE) {
                qDebug() << "GDAL: to proj4 : " << theProj4;
            } else {
                qDebug() << "GDAL: to proj4 error: " << CPLGetLastErrorMsg();
                GDALClose((GDALDatasetH)poDataset);
                return false;
            }
            QString srsProj = QString(theProj4);
            if (!srsProj.isEmpty() && theProjection != srsProj) {
                cleanup();
                theProjection = srsProj;
            }
            isLatLon = (theSrs->IsGeographic() == TRUE);
        }
    }
    if (theProjection.isEmpty()) {
        theProjection = ProjectionChooser::getProjection(QCoreApplication::translate("ImportExportGdal", "Unable to set projection; please specify one"));
        if (theProjection.isEmpty()) {
            GDALClose((GDALDatasetH)poDataset);
            return false;
        }
    }

    qDebug( "Driver: %s/%s\n",
            poDataset->GetDriver()->GetDescription(),
            poDataset->GetDriver()->GetMetadataItem( GDAL_DMD_LONGNAME ) );

    qDebug( "Size is %dx%dx%d\n",
            poDataset->GetRasterXSize(), poDataset->GetRasterYSize(),
            poDataset->GetRasterCount() );

    GdalAdapter::ImgType theType = GdalAdapter::Unknown;
    int bandCount = poDataset->GetRasterCount();
    int ixA = -1;
    int ixR, ixG, ixB;
    int ixH, ixS, ixL;
    int ixC, ixM, ixY, ixK;
    int ixYuvY, ixYuvU, ixYuvV;
    double adfMinMax[2];
    double UnknownUnit;
    GDALColorTable* colTable = NULL;
    for (int i=0; i<bandCount; ++i) {
        GDALRasterBand  *poBand = poDataset->GetRasterBand( i+1 );
        GDALColorInterp bandtype = poBand->GetColorInterpretation();
        qDebug() << "Band " << i+1 << " Color: " <<  GDALGetColorInterpretationName(poBand->GetColorInterpretation());

        switch (bandtype)
        {
        case GCI_Undefined:
            theType = GdalAdapter::Unknown;
            int             bGotMin, bGotMax;
            adfMinMax[0] = poBand->GetMinimum( &bGotMin );
            adfMinMax[1] = poBand->GetMaximum( &bGotMax );
            if( ! (bGotMin && bGotMax) )
                GDALComputeRasterMinMax((GDALRasterBandH)poBand, TRUE, adfMinMax);
            UnknownUnit = (adfMinMax[1] - adfMinMax[0]) / 256;
            break;
        case GCI_GrayIndex:
            theType = GdalAdapter::GrayScale;
            break;
        case GCI_RedBand:
            theType = GdalAdapter::Rgb;
            ixR = i;
            break;
        case GCI_GreenBand:
            theType = GdalAdapter::Rgb;
            ixG = i;
            break;
        case GCI_BlueBand :
            theType = GdalAdapter::Rgb;
            ixB = i;
            break;
        case GCI_HueBand:
            theType = GdalAdapter::Hsl;
            ixH = i;
            break;
        case GCI_SaturationBand:
            theType = GdalAdapter::Hsl;
            ixS = i;
            break;
        case GCI_LightnessBand:
            theType = GdalAdapter::Hsl;
            ixL = i;
            break;
        case GCI_CyanBand:
            theType = GdalAdapter::Cmyk;
            ixC = i;
            break;
        case GCI_MagentaBand:
            theType = GdalAdapter::Cmyk;
            ixM = i;
            break;
        case GCI_YellowBand:
            theType = GdalAdapter::Cmyk;
            ixY = i;
            break;
        case GCI_BlackBand:
            theType = GdalAdapter::Cmyk;
            ixK = i;
            break;
        case GCI_YCbCr_YBand:
            theType = GdalAdapter::YUV;
            ixYuvY = i;
            break;
        case GCI_YCbCr_CbBand:
            theType = GdalAdapter::YUV;
            ixYuvU = i;
            break;
        case GCI_YCbCr_CrBand:
            theType = GdalAdapter::YUV;
            ixYuvV = i;
            break;
        case GCI_AlphaBand:
            ixA = i;
            break;
        case GCI_PaletteIndex:
            colTable = poBand->GetColorTable();
            switch (colTable->GetPaletteInterpretation())
            {
            case GPI_Gray :
                theType = GdalAdapter::Palette_Gray;
                break;
            case GPI_RGB :
                theType = GdalAdapter::Palette_RGBA;
                break;
            case GPI_CMYK :
                theType = GdalAdapter::Palette_CMYK;
                break;
            case GPI_HLS :
                theType = GdalAdapter::Palette_HLS;
                break;
            }
            break;
        }
    }

    QSize theImgSize(poDataset->GetRasterXSize(), poDataset->GetRasterYSize());
    QImage theImg = QImage(theImgSize, QImage::Format_ARGB32);

    // Make sure that lineBuf holds one whole line of data.
    float *lineBuf;
    lineBuf = (float *) CPLMalloc(theImgSize.width() * bandCount * sizeof(float));

    int px, py;
    //every row loop
    for (int row = 0; row < theImgSize.height(); row++) {
        py = row;
        CPLErr err = poDataset->RasterIO( GF_Read, 0, row, theImgSize.width(),
                1, lineBuf, theImgSize.width(), 1, GDT_Float32, bandCount,
                NULL, sizeof(float) * bandCount, 0, sizeof(float) );
        /* FIXME: Perhaps break, or check if more work needs to be done
         * (filling with an error pattern? */
        if (err != CE_None) {
            qDebug() << "RasterIO failed to read row. Skipping.";
            continue;
        }

        // every pixel in row.
        for (int col = 0; col < theImgSize.width(); col++){
            px = col;
            switch (theType)
            {
            case GdalAdapter::Unknown:
            {
                float* v = lineBuf + (col*bandCount);
                float val = (*v - adfMinMax[0]) / UnknownUnit;
                theImg.setPixel(px, py, qRgb(val, val, val));
                break;
            }
            case GdalAdapter::GrayScale:
            {
                float* v = lineBuf + (col*bandCount);
                theImg.setPixel(px, py, qRgb(*v, *v, *v));
                break;
            }
            case GdalAdapter::Rgb:
            {
                float* r = lineBuf + (col*bandCount) + ixR;
                float* g = lineBuf + (col*bandCount) + ixG;
                float* b = lineBuf + (col*bandCount) + ixB;
                int a = 255;
                if (ixA != -1) {
                    float* fa = lineBuf + (col*bandCount) + ixA;
                    a = *fa;
                }
                theImg.setPixel(px, py, qRgba(*r, *g, *b, a));
                break;
            }
#if QT_VERSION >= 0x040600
            case GdalAdapter::Hsl:
            {
                float* h = lineBuf + (col*bandCount) + ixH;
                float* s = lineBuf + (col*bandCount) + ixS;
                float* l = lineBuf + (col*bandCount) + ixL;
                int a = 255;
                if (ixA != -1) {
                    float* fa = lineBuf + (col*bandCount) + ixA;
                    a = *fa;
                }
                QColor C = QColor::fromHsl(*h, *s, *l, a);
                theImg.setPixel(px, py, C.rgba());
                break;
            }
#endif
            case GdalAdapter::Cmyk:
            {
                float* c = lineBuf + (col*bandCount) + ixC;
                float* m = lineBuf + (col*bandCount) + ixM;
                float* y = lineBuf + (col*bandCount) + ixY;
                float* k = lineBuf + (col*bandCount) + ixK;
                int a = 255;
                if (ixA != -1) {
                    float* fa = lineBuf + (col*bandCount) + ixA;
                    a = *fa;
                }
                QColor C = QColor::fromCmyk(*c, *m, *y, *k, a);
                theImg.setPixel(px, py, C.rgba());
                break;
            }
            case GdalAdapter::YUV:
            {
                // From http://www.fourcc.org/fccyvrgb.php
                float* y = lineBuf + (col*bandCount) + ixYuvY;
                float* u = lineBuf + (col*bandCount) + ixYuvU;
                float* v = lineBuf + (col*bandCount) + ixYuvV;
                int a = 255;
                if (ixA != -1) {
                    float* fa = lineBuf + (col*bandCount) + ixA;
                    a = *fa;
                }
                float R = 1.164*(*y - 16) + 1.596*(*v - 128);
                float G = 1.164*(*y - 16) - 0.813*(*v - 128) - 0.391*(*u - 128);
                float B = 1.164*(*y - 16) + 2.018*(*u - 128);

                theImg.setPixel(px, py, qRgba(R, G, B, a));
                break;
            }
            case GdalAdapter::Palette_Gray:
            {
                float* ix = (lineBuf + (col*bandCount));
                const GDALColorEntry* color = colTable->GetColorEntry(*ix);
                theImg.setPixel(px, py, qRgb(color->c1, color->c1, color->c1));
                break;
            }
            case GdalAdapter::Palette_RGBA:
            {
                float* ix = (lineBuf + (col*bandCount));
                const GDALColorEntry* color = colTable->GetColorEntry(*ix);
                theImg.setPixel(px, py, qRgba(color->c1, color->c2, color->c3, color->c4));
                break;
            }
#if QT_VERSION >= 0x040600
            case GdalAdapter::Palette_HLS:
            {
                float* ix = (lineBuf + (col*bandCount));
                const GDALColorEntry* color = colTable->GetColorEntry(*ix);
                QColor C = QColor::fromHsl(color->c1, color->c2, color->c3, color->c4);
                theImg.setPixel(px, py, C.rgba());
                break;
            }
#endif
            case GdalAdapter::Palette_CMYK:
            {
                float* ix = (lineBuf + (col*bandCount));
                const GDALColorEntry* color = colTable->GetColorEntry(*ix);
                QColor C = QColor::fromCmyk(color->c1, color->c2, color->c3, color->c4);
                theImg.setPixel(px, py, C.rgba());
                break;
            }
            }
        }
        QCoreApplication::processEvents();
    }

    img.theFilename = fn;
    img.theImg = QPixmap::fromImage(theImg);
    theImages.push_back(img);
    theBbox = theBbox.united(bbox);

    GDALClose((GDALDatasetH)poDataset);
    return true;
}

void GdalAdapter::onLoadImage()
{
    int fileOk = 0;

    QStringList fileNames = QFileDialog::getOpenFileNames(
                    NULL,
                    tr("Open GDAL files"),
                    "", FILTER_OPEN_SUPPORTED);
    if (fileNames.isEmpty())
        return;

//    theBbox = QRectF();
//    theImages.clear();

    for (int i=0; i<fileNames.size(); i++) {
        if (loadImage(fileNames[i]))
            ++fileOk;
    }

    if (!fileOk) {
        QMessageBox::critical(0,QCoreApplication::translate("GdalBackground","No valid file"),QCoreApplication::translate("GdalBackground","No valid GDAL file could be found."));
    } else {
        emit forceZoom();
        emit forceRefresh();
    }

    return;
}

void GdalAdapter::onSetSourceTag()
{
    bool ok;
    QString text = QInputDialog::getText(0, tr("Please specify automatic \"source\" tag value"),
                                         tr("Value:"), QLineEdit::Normal, theSourceTag, &ok);
    if (ok)
        theSourceTag = text;
}

QString GdalAdapter::getSourceTag() const
{
    return theSourceTag;
}

QString	GdalAdapter::getHost() const
{
    return QString();
}

IMapAdapter::Type GdalAdapter::getType() const
{
    return IMapAdapter::DirectBackground;
}

QMenu* GdalAdapter::getMenu() const
{
    return theMenu;
}

QRectF GdalAdapter::getBoundingbox() const
{
    QRectF projBbox = theBbox;
    if (isLatLon)
        projBbox = QRectF(angToRad(theBbox.left()), angToRad(theBbox.top()), angToRad(theBbox.width()), angToRad(theBbox.height()));
    return projBbox;
}

QString GdalAdapter::projection() const
{
    return theProjection;
}

QPixmap GdalAdapter::getPixmap(const QRectF& /*wgs84Bbox*/, const QRectF& theProjBbox, const QRect& src) const
{
    QPixmap pix(src.size());
    pix.fill(Qt::transparent);
    QPainter p(&pix);

    QRectF projBbox = theProjBbox;
    if (isLatLon)
        projBbox = QRectF(radToAng(theProjBbox.left()), radToAng(theProjBbox.top()), radToAng(theProjBbox.width()), radToAng(theProjBbox.height()));

    for (int i=0; i<theImages.size(); ++i) {
        QPixmap theImg = theImages[i].theImg;

        QSizeF sz(projBbox.width() / theImages[i].adfGeoTransform[1], projBbox.height() / theImages[i].adfGeoTransform[5]);
        if (sz.isNull())
            return QPixmap();

        QPointF s((projBbox.left() - theImages[i].adfGeoTransform[0]) / theImages[i].adfGeoTransform[1],
                 (projBbox.top() - theImages[i].adfGeoTransform[3]) / theImages[i].adfGeoTransform[5]);

        qDebug() << "Pixmap Origin: " << s.x() << "," << s.y();
        qDebug() << "Pixmap size: " << sz.width() << "," << sz.height();

        double rtx = src.width() / (double)sz.width();
        double rty = src.height() / (double)sz.height();

        QRect mRect = QRect(s.toPoint(), sz.toSize());
        QRect iRect = theImg.rect().intersected(mRect);
        QRect sRect = QRect(iRect.topLeft() - mRect.topLeft(), iRect.size());
        QRect fRect = QRect(sRect.x() * rtx, sRect.y() * rty, sRect.width() * rtx, sRect.height() * rty);

        qDebug() << "mrect: " << mRect;
        qDebug() << "iRect: " << iRect;
        qDebug() << "sRect: " << sRect;

    //	QImage img2 = theImg.copy(iRect).scaled(fRect.size());
    //	p.drawImage(fRect.topLeft(), img2);
        QPixmap img2 = theImg.copy(iRect).scaled(fRect.size());
        p.drawPixmap(fRect.topLeft(), img2);
    }

    p.end();
    return pix;
}

IImageManager* GdalAdapter::getImageManager()
{
    return NULL;
}

void GdalAdapter::setImageManager(IImageManager* /*anImageManager*/)
{
}

void GdalAdapter::cleanup()
{
    theImages.clear();
    theBbox = QRectF();
    theProjection = QString();
}

bool GdalAdapter::toXML(QXmlStreamWriter& stream)
{
    bool OK = true;

    stream.writeStartElement("Images");

    stream.writeAttribute("projection", theProjection);
    if (!theSourceTag.isEmpty())
        stream.writeAttribute("source", theSourceTag);
    for (int i=0; i<theImages.size(); ++i) {
        stream.writeStartElement("Image");
        stream.writeAttribute("filename", theImages[i].theFilename);
        stream.writeEndElement();
    }

    stream.writeEndElement();

    return OK;
}

void GdalAdapter::fromXML(QXmlStreamReader& stream)
{
    theBbox = QRectF();
    theImages.clear();

    while(!stream.atEnd() && !stream.isEndElement()) {
        if (stream.name() == "Images") {
            if (stream.attributes().hasAttribute("projection"))
                theProjection = stream.attributes().value("projection").toString();
            if (stream.attributes().hasAttribute("source"))
                theSourceTag = stream.attributes().value("source").toString();
            stream.readNext();
            while(!stream.atEnd() && !stream.isEndElement()) {
                if (stream.name() == "Image") {
                    QString fn = stream.attributes().value("filename").toString();
                    if (!fn.isEmpty())
                        loadImage(fn);
                    stream.readNext();
                } else if (!stream.isWhitespace()) {
                    qDebug() << "gdalimage: logic error: " << stream.name() << " : " << stream.tokenType() << " (" << stream.lineNumber() << ")";
                    stream.skipCurrentElement();
                }
                stream.readNext();
            }
        } else if (!stream.isWhitespace()) {
            qDebug() << "gdalmain: logic error: " << stream.name() << " : " << stream.tokenType() << " (" << stream.lineNumber() << ")";
            stream.skipCurrentElement();
        }

        stream.readNext();
    }
}

QString GdalAdapter::toPropertiesHtml()
{
    QString h;

    QStringList fn;
    for (int i=0; i<theImages.size(); ++i) {
        fn << QDir::toNativeSeparators(theImages[i].theFilename);
    }
    h += "<i>" + tr("Filename(s)") + ": </i>" + fn.join("; ");

    return h;
}

#if !(QT_VERSION >= QT_VERSION_CHECK(5,0,0))
Q_EXPORT_PLUGIN2(MGdalBackgroundPlugin, GdalAdapterFactory)
#endif

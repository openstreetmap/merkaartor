//***************************************************************
// CLass: GeoTiffAdapter
//
// Description:
//
//
// Author: Chris Browet <cbro@semperpax.com> (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//******************************************************************

#include "GeoTiffAdapter.h"

#include <QCoreApplication>
#include <QtPlugin>
#include <QAction>
#include <QFileDialog>
#include <QPainter>
#include <QMessageBox>

#include <QDebug>

#include "gdal_priv.h"
#include "ogrsf_frmts.h"

#define IN_MEMORY_LIMIT 100000000

static const QUuid theUid ("{867e78e9-3156-45f8-a9a7-e5cfa52f8507}");
static const QString theName("GeoTIFF");

QUuid GeoTiffAdapterFactory::getId() const
{
    return theUid;
}

QString	GeoTiffAdapterFactory::getName() const
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
    tr("Supported formats")+" (*.tif *.tiff)\n" \
    +tr("GeoTIFF files (*.tif *.tiff)\n") \
    +tr("All Files (*)")

GeoTiffAdapter::GeoTiffAdapter()
    : poDataset(0), isLatLon(false)
{
    GDALAllRegister();

    QAction* loadImage = new QAction(tr("Load image(s)..."), this);
    loadImage->setData(theUid.toString());
    connect(loadImage, SIGNAL(triggered()), SLOT(onLoadImage()));
    theMenu = new QMenu();
    theMenu->addAction(loadImage);
}


GeoTiffAdapter::~GeoTiffAdapter()
{
}

QUuid GeoTiffAdapter::getId() const
{
    return theUid;
}

QString	GeoTiffAdapter::getName() const
{
    return theName;
}

bool GeoTiffAdapter::alreadyLoaded(QString fn) const
{
    for (int j=0; j<theImages.size(); ++j)
        if (theImages[j].theFilename == fn)
            return true;
    return false;
}

bool GeoTiffAdapter::loadImage(const QString& fn)
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

    isLatLon = false;
    if( strlen(poDataset->GetProjectionRef()) != 0 ) {
        qDebug( "Projection is `%s'\n", poDataset->GetProjectionRef() );
        OGRSpatialReference* theSrs = new OGRSpatialReference(poDataset->GetProjectionRef());
        if (theSrs) {
            theSrs->morphFromESRI();
            char* theProj4;
            if (theSrs->exportToProj4(&theProj4) == OGRERR_NONE) {
                qDebug() << "GDAL: to proj4 : " << theProj4;
            } else {
                qDebug() << "GDAL: to proj4 error: " << CPLGetLastErrorMsg();
                return false;
            }
            theProjection = QString(theProj4);
            isLatLon = (theSrs->IsGeographic() == TRUE);
        }
    } else
        return false;

    if( poDataset->GetGeoTransform( img.adfGeoTransform ) == CE_None )
    {
        qDebug( "Origin = (%.6f,%.6f)\n",
                img.adfGeoTransform[0], img.adfGeoTransform[3] );

        qDebug( "Pixel Size = (%.6f,%.6f)\n",
                img.adfGeoTransform[1], img.adfGeoTransform[5] );

        bbox.setTopLeft(QPointF(img.adfGeoTransform[0], img.adfGeoTransform[3]));
        bbox.setWidth(img.adfGeoTransform[1]*poDataset->GetRasterXSize());
        bbox.setHeight(img.adfGeoTransform[5]*poDataset->GetRasterYSize());
    } else
        return false;

    qDebug( "Driver: %s/%s\n",
            poDataset->GetDriver()->GetDescription(),
            poDataset->GetDriver()->GetMetadataItem( GDAL_DMD_LONGNAME ) );

    qDebug( "Size is %dx%dx%d\n",
            poDataset->GetRasterXSize(), poDataset->GetRasterYSize(),
            poDataset->GetRasterCount() );

    img.theFilename = fn;
    img.theImg.load(fn);
    theImages.push_back(img);
    theBbox = theBbox.united(bbox);

    return true;
}

void GeoTiffAdapter::onLoadImage()
{
    int fileOk = 0;

    QStringList fileNames = QFileDialog::getOpenFileNames(
                    NULL,
                    tr("Open GeoTIFF files"),
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
        QMessageBox::critical(0,QCoreApplication::translate("GeoTiffBackground","No valid file"),QCoreApplication::translate("GeoTiffBackground","No valid GeoTIFF file could be found."));
    } else {
        emit forceZoom();
    }

    return;
}

QString	GeoTiffAdapter::getHost() const
{
    return "";
}

IMapAdapter::Type GeoTiffAdapter::getType() const
{
    return IMapAdapter::DirectBackground;
}

QMenu* GeoTiffAdapter::getMenu() const
{
    return theMenu;
}

QRectF GeoTiffAdapter::getBoundingbox() const
{
    QRectF projBbox = theBbox;
    if (isLatLon)
        projBbox = QRectF(angToRad(theBbox.left()), angToRad(theBbox.top()), angToRad(theBbox.width()), angToRad(theBbox.height()));
    return projBbox;
}

QString GeoTiffAdapter::projection() const
{
    return theProjection;
}

QPixmap GeoTiffAdapter::getPixmap(const QRectF& /*wgs84Bbox*/, const QRectF& theProjBbox, const QRect& src) const
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
        QRect iRect = theImg.rect().intersect(mRect);
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

IImageManager* GeoTiffAdapter::getImageManager()
{
    return NULL;
}

void GeoTiffAdapter::setImageManager(IImageManager* /*anImageManager*/)
{
}

void GeoTiffAdapter::cleanup()
{
    theImages.clear();
    theBbox = QRectF();
    theProjection = QString();
}

bool GeoTiffAdapter::toXML(QDomElement xParent)
{
    bool OK = true;

    QDomElement fs = xParent.ownerDocument().createElement("Images");
    xParent.appendChild(fs);

    for (int i=0; i<theImages.size(); ++i) {
        QDomElement f = xParent.ownerDocument().createElement("Image");
        fs.appendChild(f);
        f.setAttribute("filename", theImages[i].theFilename);
    }

    return OK;
}

void GeoTiffAdapter::fromXML(const QDomElement xParent)
{
    theBbox = QRectF();
    theImages.clear();

    QDomElement fs = xParent.firstChildElement();
    while(!fs.isNull()) {
        if (fs.tagName() == "Images") {
            QDomElement f = fs.firstChildElement();
            while(!f.isNull()) {
                if (f.tagName() == "Image") {
                    QString fn = f.attribute("filename");
                    if (!fn.isEmpty())
                        loadImage(fn);
                }
                f = f.nextSiblingElement();
            }
        }

        fs = fs.nextSiblingElement();
    }
}

QString GeoTiffAdapter::toPropertiesHtml()
{
    QString h;

    QStringList fn;
    for (int i=0; i<theImages.size(); ++i) {
        fn << QDir::toNativeSeparators(theImages[i].theFilename);
    }
    h += "<i>" + tr("Filename(s)") + ": </i>" + fn.join("; ");

    return h;
}

Q_EXPORT_PLUGIN2(MGeoTiffBackgroundPlugin, GeoTiffAdapterFactory)

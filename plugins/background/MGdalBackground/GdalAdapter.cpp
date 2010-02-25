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

#include "gdal_priv.h"
#include "ogrsf_frmts.h"

#define IN_MEMORY_LIMIT 100000000

static const QUuid theUid ("{867e78e9-3156-45f8-a9a7-e5cfa52f8507}");

#define FILTER_OPEN_SUPPORTED \
    tr("Supported formats")+" (*.tif *.tiff)\n" \
    +tr("GeoTIFF files (*.tif *.tiff)\n") \
    +tr("All Files (*)")

GdalAdapter::GdalAdapter()
    : poDataset(0)
{
    GDALAllRegister();

    QAction* loadImage = new QAction(tr("Load image..."), this);
    loadImage->setData(theUid.toString());
    connect(loadImage, SIGNAL(triggered()), SLOT(onLoadImage()));
    theMenu = new QMenu();
    theMenu->addAction(loadImage);
}


GdalAdapter::~GdalAdapter()
{
}

void GdalAdapter::onLoadImage()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(
                    NULL,
                    tr("Open GDAL files"),
                    "", FILTER_OPEN_SUPPORTED);
    if (fileNames.isEmpty())
        return;

    theImages.clear();

    for (int i=0; i<fileNames.size(); i++) {
        QFileInfo fi(fileNames[i]);
        GdalImage img;

        poDataset = (GDALDataset *) GDALOpen( QDir::toNativeSeparators(fi.absoluteFilePath()).toUtf8().constData(), GA_ReadOnly );
        if( poDataset == NULL )
        {
            qDebug() <<  "GDAL Open failed: " << fileNames[i];
            continue;
        }

        if( poDataset->GetProjectionRef()  != NULL ) {
            qDebug( "Projection is `%s'\n", poDataset->GetProjectionRef() );
            OGRSpatialReference* theSrs = new OGRSpatialReference(poDataset->GetProjectionRef());
            if (theSrs) {
                theSrs->morphFromESRI();
                char* theProj4;
                if (theSrs->exportToProj4(&theProj4) == OGRERR_NONE) {
                    qDebug() << "SHP: to proj4 : " << theProj4;
                } else {
                    qDebug() << "SHP: to proj4 error: " << CPLGetLastErrorMsg();
                    return;
                }
                theProjection = QString(theProj4);
            }
        }

        if( poDataset->GetGeoTransform( img.adfGeoTransform ) == CE_None )
        {
            qDebug( "Origin = (%.6f,%.6f)\n",
                    img.adfGeoTransform[0], img.adfGeoTransform[3] );

            qDebug( "Pixel Size = (%.6f,%.6f)\n",
                    img.adfGeoTransform[1], img.adfGeoTransform[5] );
        }

        qDebug( "Driver: %s/%s\n",
                poDataset->GetDriver()->GetDescription(),
                poDataset->GetDriver()->GetMetadataItem( GDAL_DMD_LONGNAME ) );

        qDebug( "Size is %dx%dx%d\n",
                poDataset->GetRasterXSize(), poDataset->GetRasterYSize(),
                poDataset->GetRasterCount() );

        img.theImg.load(fileNames[i]);
        theImages.push_back(img);
    }

//	theType == GdalAdapter::Unknown;
//	bandCount = poDataset->GetRasterCount();
//	for (int i=0; i<bandCount; ++i) {
//		GDALRasterBand  *poBand = poDataset->GetRasterBand( i+1 );
//		GDALColorInterp bandtype = poBand->GetColorInterpretation();
//		qDebug() << "Band " << i+1 << " Color: " <<  GDALGetColorInterpretationName(poBand->GetColorInterpretation());
//
//		switch (bandtype)
//		{
//			case GCI_RedBand:
//				theType = GdalAdapter::Rgb;
//				ixR = i;
//				break;
//			case GCI_GreenBand:
//				theType = GdalAdapter::Rgb;
//				ixG = i;
//				break;
//			case GCI_BlueBand :
//				theType = GdalAdapter::Rgb;
//				ixB = i;
//				break;
//			case GCI_AlphaBand:
//				if (theType == GdalAdapter::Rgb)
//					theType = GdalAdapter::Rgba;
//				ixA = i;
//				break;
//			case GCI_PaletteIndex:
//				colTable = poBand->GetColorTable();
//				switch (colTable->GetPaletteInterpretation())
//				{
//					case GPI_Gray :
//						theType = GdalAdapter::Palette_Gray;
//						break;
//					case GPI_RGB :
//						theType = GdalAdapter::Palette_RGBA;
//						break;
//					case GPI_CMYK :
//						theType = GdalAdapter::Palette_CMYK;
//						break;
//					case GPI_HLS :
//						theType = GdalAdapter::Palette_HLS;
//						break;
//				}
//				break;
//		}
//	}

//	theImg = QImage(theImgRect.size(), QImage::Format_ARGB32);
//
//	// Make sure that lineBuf holds one whole line of data.
//	float *lineBuf;
//	lineBuf = (float *) CPLMalloc(theImgRect.width() * bandCount * sizeof(float));
//
//	int px, py;
//	//every row loop
//	for (int row = 0; row < theImgRect.height(); row++) {
//		py = row;
//		poDataset->RasterIO( GF_Read, theImgRect.left(), theImgRect.top() + row, theImgRect.width(), 1, lineBuf, theImgRect.width(), 1, GDT_Float32,
//							 bandCount, NULL, sizeof(float) * bandCount, 0, sizeof(float) );
//		// every pixel in row.
//		for (int col = 0; col < theImgRect.width(); col++){
//			px = col;
//			switch (theType)
//			{
//				case GdalAdapter::Rgb:
//				{
//					float* r = lineBuf + (col*bandCount) + ixR;
//					float* g = lineBuf + (col*bandCount) + ixG;
//					float* b = lineBuf + (col*bandCount) + ixB;
//					theImg.setPixel(px, py, qRgb(*r, *g, *b));
//					break;
//				}
//				case GdalAdapter::Rgba:
//				{
//					float* r = lineBuf + (col*bandCount) + ixR;
//					float* g = lineBuf + (col*bandCount) + ixG;
//					float* b = lineBuf + (col*bandCount) + ixB;
//					float* a = lineBuf + (col*bandCount) + ixA;
//					theImg.setPixel(px, py, qRgba(*r, *g, *b, *a));
//					break;
//				}
//			case GdalAdapter::Palette_RGBA:
//				{
//					float* ix = (lineBuf + (col*bandCount));
//					const GDALColorEntry* color = colTable->GetColorEntry(*ix);
//					theImg.setPixel(px, py, qRgba(color->c1, color->c2, color->c3, color->c4));
//				}
//			}
//		}
//		QCoreApplication::processEvents();
//	}
    return;
}

QString	GdalAdapter::getHost() const
{
    return "";
}

QUuid GdalAdapter::getId() const
{
    return QUuid(theUid);
}

IMapAdapter::Type GdalAdapter::getType() const
{
    return IMapAdapter::DirectBackground;
}

QString	GdalAdapter::getName() const
{
    return "GeoTIFF";
}

QMenu* GdalAdapter::getMenu() const
{
    return theMenu;
}

QString GdalAdapter::projection() const
{
    return theProjection;
}

QPixmap GdalAdapter::getPixmap(const QRectF& wgs84Bbox, const QRectF& projBbox, const QRect& src) const
{
    QPixmap pix(src.size());
    pix.fill(Qt::transparent);
    QPainter p(&pix);

    for (int i=0; i<theImages.size(); ++i) {
        QPixmap theImg = theImages[i].theImg;

        QSize sz(projBbox.width() / theImages[i].adfGeoTransform[1], projBbox.height() / theImages[i].adfGeoTransform[5]);
        if (sz.isNull())
            return QPixmap();

        QPoint s((projBbox.left() - theImages[i].adfGeoTransform[0]) / theImages[i].adfGeoTransform[1],
                 (projBbox.top() - theImages[i].adfGeoTransform[3]) / theImages[i].adfGeoTransform[5]);

        qDebug() << "Pixmap Origin: " << s.x() << "," << s.y();
        qDebug() << "Pixmap size: " << sz.width() << "," << sz.height();

        double rtx = src.width() / (double)sz.width();
        double rty = src.height() / (double)sz.height();

        QRect mRect = QRect(s, sz);
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

IImageManager* GdalAdapter::getImageManager()
{
    return NULL;
}

void GdalAdapter::setImageManager(IImageManager* anImageManager)
{
}

Q_EXPORT_PLUGIN2(MGdalBackgroundPlugin, GdalAdapter)

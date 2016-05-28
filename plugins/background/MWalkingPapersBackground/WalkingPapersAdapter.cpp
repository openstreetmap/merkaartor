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

#include "WalkingPapersAdapter.h"

#include <QCoreApplication>
#include <QtPlugin>
#include <QAction>
#include <QFileDialog>
#include <QPainter>
#include <QMessageBox>
#include <QInputDialog>
#include <QTimer>
#include <QImage>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <QDebug>

#include <math.h>

#ifdef USE_ZBAR
#include <zbar.h>
#include <zbar/QZBarImage.h>
#endif

// from wikipedia
#define EQUATORIALRADIUS 6378137.0
#define POLARRADIUS      6356752.0
#define EQUATORIALMETERCIRCUMFERENCE  40075016.68
#define EQUATORIALMETERHALFCIRCUMFERENCE  20037508.34
#define EQUATORIALMETERPERDEGREE    222638.981555556

static const QUuid theUid ("{c580b2bc-dd14-40b2-8bb6-241da2a1fdb3}");
static const QString theName("Walking Papers");

QUuid WalkingPapersAdapterFactory::getId() const
{
    return theUid;
}

QString	WalkingPapersAdapterFactory::getName() const
{
    return theName;
}

/**************/


#define FILTER_OPEN_SUPPORTED \
    tr("Supported formats")+" (*.jpg *.png *.bmp)\n" \
    +tr("All Files (*)")

double angToRad(double a)
{
    return a*M_PI/180.;
}

QPointF mercatorProject(const QPointF& c)
{
    double x = angToRad(c.x()) / M_PI * EQUATORIALMETERHALFCIRCUMFERENCE;
    double y = log(tan(angToRad(c.y())) + 1/cos(angToRad(c.y()))) / M_PI * (EQUATORIALMETERHALFCIRCUMFERENCE);

    return QPointF(x, y);
}

WalkingPapersAdapter::WalkingPapersAdapter()
    : theImageManager(0)
{
    QAction* loadImage = new QAction(tr("Load image..."), this);
    loadImage->setData(theUid.toString());
    connect(loadImage, SIGNAL(triggered()), SLOT(onLoadImage()));
    theMenu = new QMenu();
    theMenu->addAction(loadImage);
}


WalkingPapersAdapter::~WalkingPapersAdapter()
{
}

QUuid WalkingPapersAdapter::getId() const
{
    return theUid;
}

QString	WalkingPapersAdapter::getName() const
{
    return theName;
}

bool WalkingPapersAdapter::alreadyLoaded(QString fn) const
{
    for (int j=0; j<theImages.size(); ++j)
        if (theImages[j].theFilename == fn)
            return true;
    return false;
}

void make_grayscale(QImage& in)
{
    if(in.format()!=QImage::Format_Indexed8)
        throw "format error";
    QVector<int> transform_table(in.colorCount());
    for(int i=0;i<in.colorCount();i++)
    {
        QRgb c1=in.color(i);
        int avg=qGray(c1);
        transform_table[i] = avg;
    }
    in.setColorCount(256);
    for(int i=0;i<256;i++)
        in.setColor(i,qRgb(i,i,i));
    for(int i=0;i<in.byteCount();i++)
    {
        in.bits()[i]=transform_table[in.bits()[i]];
    }
}

bool WalkingPapersAdapter::getWalkingPapersDetails(const QUrl& reqUrl, QRectF& bbox) const
{
    QNetworkAccessManager* manager = theImageManager->getNetworkManager();
    QEventLoop q;
    QTimer tT;

    if (!reqUrl.host().contains("walking-papers.org"))
        return false;

    tT.setSingleShot(true);
    connect(&tT, SIGNAL(timeout()), &q, SLOT(quit()));
    connect(manager, SIGNAL(finished(QNetworkReply*)),
            &q, SLOT(quit()));
    QNetworkReply *reply = manager->get(QNetworkRequest(reqUrl));

    tT.start(theSets->value("Network/NetworkTimeout", 5000).toInt());
    q.exec();
    if(tT.isActive()) {
        // download complete
        tT.stop();
    } else {
        QMessageBox::warning(0, tr("Network timeout"), tr("Cannot read the photo's details from the Walking Papers server."), QMessageBox::Ok);
        return false;
    }

    QString center = QString::fromLatin1(reply->rawHeader("X-Print-Bounds"));
    QStringList sl = center.split(" ");
    if (!sl.size() == 4)
        return false;

    QPointF tl(sl[1].toDouble(), sl[0].toDouble());
    QPointF br(sl[3].toDouble(), sl[2].toDouble());

    qDebug() << tl << "; " << br;

    bbox = QRectF(tl, br);

    return true;
}

bool WalkingPapersAdapter::askAndgetWalkingPapersDetails(QRectF& bbox) const
{
    bool ok;
    QString text = QInputDialog::getText(0, tr("Please specify Walking Papers URL"),
                                         tr("URL:"), QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty()) {
        QUrl url(text);
        return getWalkingPapersDetails(url, bbox);
    } else
        return false;
}

bool WalkingPapersAdapter::loadImage(const QString& fn, QRectF theBbox, int theRotation)
{
    if (alreadyLoaded(fn))
        return true;

    QImage img(fn);
    WalkingPapersImage wimg;
    if (theBbox.isNull()) {
#ifdef USE_ZBAR
        zbar::QZBarImage image(img);

        // create a reader
        zbar::ImageScanner scanner;

        // configure the reader
        scanner.set_config(zbar::ZBAR_NONE, zbar::ZBAR_CFG_ENABLE, 1);

        // scan the image for barcodes
        scanner.recycle_image(image);
        zbar::Image tmp = image.convert(*(long*)"Y800");
        int n = scanner.scan(tmp);
        image.set_symbols(tmp.get_symbols());

        if (n <= 0) {
            qDebug() << "WP scan error: " << n;
            if (!askAndgetWalkingPapersDetails(theBbox))
                return false;
        } else {
            QUrl url;
            // extract results
            for(zbar::Image::SymbolIterator symbol = image.symbol_begin(); symbol != image.symbol_end(); ++symbol) {
                // do something useful with results
                qDebug() << "decoded " << QString::fromStdString(symbol->get_type_name())
                        << " symbol \"" << QString::fromStdString(symbol->get_data()) << '"';
                qDebug() << "x;y: " << symbol->get_location_x(0) << ", " << symbol->get_location_y(0);

                url = QUrl(QString::fromStdString(symbol->get_data()));
                getWalkingPapersDetails(url, theBbox);

                int x = symbol->get_location_x(0);
                int y = symbol->get_location_y(0);
                QPoint mid = QPoint(img.width()/2, img.height()/2);
                if (x < mid.x() || y < mid.y()) {
                    if (x < mid.x() && y < mid.y())
                        theRotation = 180;
                    else if (x > mid.x() && y < mid.y())
                        theRotation = 90;
                    else if (x < mid.x() && y > mid.y())
                        theRotation = -90;
                }
            }
        }

        // clean up
        image.set_data(NULL, 0);
#else
        if (!askAndgetWalkingPapersDetails(theBbox))
            return false;
#endif
    }
    if (theRotation) {
        QMatrix mat;
        mat.rotate(theRotation);
        img = img.transformed(mat);
    }
    wimg.theFilename = fn;
    wimg.theImg = QPixmap::fromImage(img);
    wimg.theBBox = theBbox;
    wimg.rotation = theRotation;
    theImages.push_back(wimg);

    theCoordBbox |= theBbox;

    return true;
}

void WalkingPapersAdapter::onLoadImage()
{
    int fileOk = 0;

    QStringList fileNames = QFileDialog::getOpenFileNames(
                    NULL,
                    tr("Open Walking Papers scan"),
                    "", FILTER_OPEN_SUPPORTED);
    if (fileNames.isEmpty())
        return;

    QRectF theBbox = QRectF();
    for (int i=0; i<fileNames.size(); i++) {
        if (loadImage(fileNames[i], theBbox))
            ++fileOk;
    }

    if (!fileOk) {
        QMessageBox::critical(0,QCoreApplication::translate("WalkingPapersBackground","No valid file"),QCoreApplication::translate("WalkingPapersBackground","Cannot load file."));
    } else {
        emit forceProjection();
        emit forceZoom();
        emit forceRefresh();
    }

    return;
}

QString	WalkingPapersAdapter::getHost() const
{
    return QString();
}

IMapAdapter::Type WalkingPapersAdapter::getType() const
{
    return IMapAdapter::DirectBackground;
}

QMenu* WalkingPapersAdapter::getMenu() const
{
    return theMenu;
}

QRectF WalkingPapersAdapter::getBoundingbox() const
{
    QRectF projBBox;
    projBBox = QRectF(mercatorProject(theCoordBbox.topLeft()), mercatorProject(theCoordBbox.bottomRight()));
    return projBBox;
}

QString WalkingPapersAdapter::projection() const
{
    return "EPSG:900913";
}

QPixmap WalkingPapersAdapter::getPixmap(const QRectF& wgs84Bbox, const QRectF& /*projBbox*/, const QRect& src) const
{
    QPixmap pix(src.size());
    pix.fill(Qt::transparent);
    QPainter p(&pix);

    for (int i=0; i<theImages.size(); ++i) {
        QPixmap theImg = theImages[i].theImg;

        double rx = wgs84Bbox.width() / src.width();
        double ry = wgs84Bbox.height() / src.height();
        qDebug() << "rx: " << rx << "; ry: " << ry;

        QSize sz(theImages[i].theBBox.width() / rx, theImages[i].theBBox.height() / ry);
        if (sz.isNull())
            return QPixmap();

        QPoint s((theImages[i].theBBox.bottomLeft().x() - wgs84Bbox.bottomLeft().x()) / rx, (wgs84Bbox.bottomLeft().y() - theImages[i].theBBox.bottomLeft().y()) / ry);

        qDebug() << "Viewport: " << wgs84Bbox;
        qDebug() << "Pixmap Origin: " << s.x() << "," << s.y();
        qDebug() << "Pixmap size: " << sz.width() << "," << sz.height();

        double rtx = theImg.width() / (double)sz.width();
        double rty = theImg.height() / (double)sz.height();

        QRect mRect = QRect(s, sz);
        QRect iRect = theImg.rect().intersected(mRect);
        QRect sRect = QRect(iRect.topLeft() - mRect.topLeft(), iRect.size());
        QRect fRect = QRect(sRect.x() * rtx, sRect.y() * rty, sRect.width() * rtx, sRect.height() * rty);

        qDebug() << "mrect: " << mRect;
        qDebug() << "iRect: " << iRect;
        qDebug() << "sRect: " << sRect;
        qDebug() << "fRect: " << fRect;

        QPixmap img2 = theImg.copy(fRect).scaled(sRect.size());
        p.drawPixmap(iRect.topLeft(), img2);
    }

    p.end();
    return pix;
}

IImageManager* WalkingPapersAdapter::getImageManager()
{
    return theImageManager;
}

void WalkingPapersAdapter::setImageManager(IImageManager* anImageManager)
{
    theImageManager = anImageManager;
}

void WalkingPapersAdapter::cleanup()
{
    theImages.clear();
    theCoordBbox = QRectF();
}

bool WalkingPapersAdapter::toXML(QXmlStreamWriter& stream)
{
    bool OK = true;

    stream.writeStartElement("Images");
    for (int i=0; i<theImages.size(); ++i) {
        stream.writeStartElement("Image");
        stream.writeAttribute("filename", theImages[i].theFilename);
        stream.writeAttribute("top", QString::number(theImages[i].theBBox.top()));
        stream.writeAttribute("left", QString::number(theImages[i].theBBox.left()));
        stream.writeAttribute("width", QString::number(theImages[i].theBBox.width()));
        stream.writeAttribute("height", QString::number(theImages[i].theBBox.height()));
        stream.writeAttribute("rotation", QString::number(theImages[i].rotation));
        stream.writeEndElement();
    }
    stream.writeEndElement();

    return OK;
}

void WalkingPapersAdapter::fromXML(QXmlStreamReader& stream)
{
    theCoordBbox = QRectF();
    theImages.clear();

    stream.readNext();
    while(!stream.atEnd() && !stream.isEndElement()) {
        if (stream.name() == "Images") {
            stream.readNext();
            while(!stream.atEnd() && !stream.isEndElement()) {
                if (stream.name() == "Image") {
                    QString fn = stream.attributes().value("filename").toString();
                    if (!fn.isEmpty()) {
                        double x = stream.attributes().value("left").toString().toDouble();
                        double y = stream.attributes().value("top").toString().toDouble();
                        double w = stream.attributes().value("width").toString().toDouble();
                        double h = stream.attributes().value("height").toString().toDouble();
                        int r = stream.attributes().value("rotation").toString().toInt();
                        QRectF bbox(x, y, w, h);
                        loadImage(fn, bbox, r);
                    }
                    stream.readNext();
                } else if (!stream.isWhitespace()) {
                    qDebug() << "wp: logic error: " << stream.name() << " : " << stream.tokenType() << " (" << stream.lineNumber() << ")";
                    stream.skipCurrentElement();
                }
                stream.readNext();
            }
        } else if (!stream.isWhitespace()) {
            qDebug() << "wp: logic error: " << stream.name() << " : " << stream.tokenType() << " (" << stream.lineNumber() << ")";
            stream.skipCurrentElement();
        }
        stream.readNext();
    }
}

QString WalkingPapersAdapter::toPropertiesHtml()
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
Q_EXPORT_PLUGIN2(MWalkingPapersBackgroundPlugin, WalkingPapersAdapterFactory)
#endif

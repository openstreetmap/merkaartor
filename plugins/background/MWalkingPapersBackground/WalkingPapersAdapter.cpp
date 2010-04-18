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

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <math.h>

#ifdef USE_ZBAR
#include <zbar.h>
#include <zbar/QZBarImage.h>
#endif

static const QUuid theUid ("{c580b2bc-dd14-40b2-8bb6-241da2a1fdb3}");

#define FILTER_OPEN_SUPPORTED \
    tr("Supported formats")+" (*.jpg *.png *.bmp)\n" \
    +tr("All Files (*)")

double angToRad(double a)
{
    return a*M_PI/180.;
}

QPointF mercatorProject(const QPointF& c)
{
    double x = angToRad(c.x()) / M_PI * 20037508.34;
    double y = log(tan(angToRad(c.y())) + 1/cos(angToRad(c.y()))) / M_PI * (20037508.34);

    return QPointF(x, y);
}

WalkingPapersAdapter::WalkingPapersAdapter()
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
    QVector<int> transform_table(in.numColors());
    for(int i=0;i<in.numColors();i++)
    {
        QRgb c1=in.color(i);
        int avg=qGray(c1);
        transform_table[i] = avg;
    }
    in.setNumColors(256);
    for(int i=0;i<256;i++)
        in.setColor(i,qRgb(i,i,i));
    for(int i=0;i<in.numBytes();i++)
    {
        in.bits()[i]=transform_table[in.bits()[i]];
    }
}

bool WalkingPapersAdapter::getWalkingPapersDetails(const QUrl& reqUrl, QRectF& bbox) const
{
    QNetworkAccessManager manager;
    QEventLoop q;
    QTimer tT;

    if (!reqUrl.host().contains("walking-papers.org"))
        return false;

    tT.setSingleShot(true);
    connect(&tT, SIGNAL(timeout()), &q, SLOT(quit()));
    connect(&manager, SIGNAL(finished(QNetworkReply*)),
            &q, SLOT(quit()));
    QNetworkReply *reply = manager.get(QNetworkRequest(reqUrl));

    tT.start(10000); // 10s timeout
    q.exec();
    if(tT.isActive()) {
        // download complete
        tT.stop();
    } else {
        QMessageBox::warning(0, tr("Network timeout"), tr("Cannot read the photo's details from the Walking Papers server."), QMessageBox::Ok);
        return false;
    }

    QString center = QString::fromAscii(reply->rawHeader("X-Print-Bounds"));
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

void WalkingPapersAdapter::onLoadImage()
{
    int fileOk = 0;
    QRectF theBbox = QRectF();

    QStringList fileNames = QFileDialog::getOpenFileNames(
                    NULL,
                    tr("Open Walking Papers scan"),
                    "", FILTER_OPEN_SUPPORTED);
    if (fileNames.isEmpty())
        return;

    for (int i=0; i<fileNames.size(); i++) {
        if (alreadyLoaded(fileNames[i]))
            continue;

        QImage img(fileNames[i]);
        WalkingPapersImage wimg;
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
                continue;
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
                    QMatrix mat;
                    if (x < mid.x() && y < mid.y())
                        mat.rotate(180);
                    else if (x > mid.x() && y < mid.y())
                        mat.rotate(90);
                    else if (x < mid.x() && y > mid.y())
                        mat.rotate(-90);
                    img = img.transformed(mat);
                }
            }
        }

        // clean up
        image.set_data(NULL, 0);
#else
        if (!askAndgetWalkingPapersDetails(theBbox))
            continue;
#endif
        wimg.theFilename = fileNames[i];
        wimg.theImg = QPixmap::fromImage(img);
        wimg.theBBox = theBbox;
        theImages.push_back(wimg);
        ++fileOk;

        theCoordBbox |= theBbox;
    }

    if (!fileOk) {
        QMessageBox::critical(0,QCoreApplication::translate("WalkingPapersBackground","No valid file"),QCoreApplication::translate("WalkingPapersBackground","Cannot load file."));
    }

    return;
}

QString	WalkingPapersAdapter::getHost() const
{
    return "";
}

QUuid WalkingPapersAdapter::getId() const
{
    return QUuid(theUid);
}

IMapAdapter::Type WalkingPapersAdapter::getType() const
{
    return IMapAdapter::DirectBackground;
}

QString	WalkingPapersAdapter::getName() const
{
    return "Walking Papers";
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

QPixmap WalkingPapersAdapter::getPixmap(const QRectF& wgs84Bbox, const QRectF& projBbox, const QRect& src) const
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
        QRect iRect = theImg.rect().intersect(mRect);
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
    return NULL;
}

void WalkingPapersAdapter::setImageManager(IImageManager* anImageManager)
{
}

void WalkingPapersAdapter::cleanup()
{
    theImages.clear();
    theCoordBbox = QRectF();
}

Q_EXPORT_PLUGIN2(MWalkingPapersBackgroundPlugin, WalkingPapersAdapter)

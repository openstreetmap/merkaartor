/***************************************************************************
 *   Copyright (C) 2010 by Chris Browet                                    *
 *   cbro@semperpax.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <QtPlugin>
#include <QDesktopServices>
#include <QPainter>
#include <QSettings>
#include <QUrl>

#include "CadastreFranceVector.h"

#include "cadastrewrapper.h"
#include "cadastredownloaddialog.h"

#include "IImageManager.h"

static const QUuid theUid ( "eaf960ce-f64b-4a16-ba52-4f28a9b1e26d");
static const QString theName("Cadastre Vectoriel (France)");

QUuid CadastreFranceVectorAdapterFactory::getId() const
{
    return theUid;
}

QString	CadastreFranceVectorAdapterFactory::getName() const
{
    return theName;
}

/**************/

CadastreFranceVectorAdapter::CadastreFranceVectorAdapter()
    : theImageManager(0), theMenu(0), theSettings(0)
    , current_zoom(0), min_zoom(0), max_zoom(6)
{
    loc = QLocale(QLocale::English);
    loc.setNumberOptions(QLocale::OmitGroupSeparator);

    Resolutions << 16 << 8. << 4. << 2 << 1.0 << 0.5 << 0.2;
}

CadastreFranceVectorAdapter::~CadastreFranceVectorAdapter()
{
}

QUuid CadastreFranceVectorAdapter::getId() const
{
    return theUid;
}

QString	CadastreFranceVectorAdapter::getName() const
{
    return theName;
}

void CadastreFranceVectorAdapter::setSettings(QSettings* aSet)
{
    theSettings = aSet;
    CadastreWrapper::instance()->setRootCacheDir(QDir(theSettings->value("backgroundImage/CacheDir").toString()));
    updateMenu();
}

QString	CadastreFranceVectorAdapter::getHost() const
{
    return "www.cadastre.gouv.fr";
}

IMapAdapter::Type CadastreFranceVectorAdapter::getType() const
{
    return IMapAdapter::DirectBackground;
}

QString CadastreFranceVectorAdapter::projection() const
{
    return m_city.projection();
}

QRectF CadastreFranceVectorAdapter::getBoundingbox() const
{
    double L = qMax(m_city.geometry().width(), m_city.geometry().height());
    QRectF bb(m_city.geometry());
    QRectF R = QRectF(QPointF(bb.center().x()-L/2, bb.center().y()-L/2),
                      QPointF(bb.center().x()+L/2, bb.center().y()+L/2));
    return R;
//    return QRectF(R.bottomLeft(), R.topRight());
}

QMenu* CadastreFranceVectorAdapter::getMenu() const
{
    return theMenu;
}

IImageManager* CadastreFranceVectorAdapter::getImageManager()
{
    return theImageManager;
}

void CadastreFranceVectorAdapter::setImageManager(IImageManager* anImageManager)
{
    theImageManager = anImageManager;
    CadastreWrapper::instance()->setNetworkManager(theImageManager->getNetworkManager());
    theImageManager->setCachePermanent(true);
}

void CadastreFranceVectorAdapter::updateMenu()
{
    delete theMenu;
    theMenu = new QMenu(0);

    QAction* grabCity = new QAction(tr("Grab City..."), this);
    connect(grabCity, SIGNAL(triggered()), SLOT(onGrabCity()));
    theMenu->addAction(grabCity);

    theMenu->addSeparator();

    QDir cache = CadastreWrapper::instance()->getCacheDir();
    QFileInfoList fl = cache.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs);
    foreach (QFileInfo fi, fl) {
        QSettings sets(fi.absoluteFilePath()+"/cache.ini", QSettings::IniFormat);
        QAction* cityAct = new QAction(sets.value("name").toString(), this);
        cityAct->setData(fi.fileName());
        theMenu->addAction(cityAct);
    }
    connect(theMenu, SIGNAL(triggered(QAction*)), SLOT(cityTriggered(QAction*)));
}

void CadastreFranceVectorAdapter::onGrabCity()
{
    if (!theImageManager)
        return;

    m_cacheFolder = QDir(theSettings->value("backgroundImage/CacheDir").toString()).absoluteFilePath("qadastre");
    CadastreDownloadDialog *dial = new CadastreDownloadDialog(this);
    dial->setModal(true);
    if (dial->exec()) {
        qDebug() << "Cool !";
        qDebug() << dial->getCityCode() << dial->getCityName() << dial->getBoundingBox();
        if (!m_cacheFolder.exists(dial->getCityName()))
            m_cacheFolder.mkdir(dial->getCityCode());
        QDir cityFolder(m_cacheFolder);
        cityFolder.cd(dial->getCityCode());

        QSettings parameters(cityFolder.absoluteFilePath("settings.ini"), QSettings::IniFormat);
        parameters.setValue("name", dial->getCityName());
        parameters.setValue("code", dial->getCityCode());
        parameters.setValue("bbox", dial->getBoundingBox());
        parameters.setValue("projection", dial->getProjection());
        parameters.sync();

        QFile pdf(cityFolder.absoluteFilePath("map.pdf"));
        pdf.open(QIODevice::WriteOnly);
        pdf.write(dial->getResultDevice()->readAll());
        pdf.close();
        dial->getResultDevice()->close();
        dial->getResultDevice()->deleteLater();

        m_code = dial->getCityCode();
        initializeCity(m_code);
    }
    dial->deleteLater();
}

void CadastreFranceVectorAdapter::cityTriggered(QAction *act)
{
    if (act->data().toString().isEmpty())
        return;
    m_code = act->data().toString();
    if (!theImageManager)
        return;
    initializeCity(m_code);
}

void CadastreFranceVectorAdapter::initializeCity(QString code)
{
    QDir folder = QDir(m_cacheFolder.absoluteFilePath(cityCode));
    QString pdfFileName = folder.absoluteFilePath("map.pdf");
    QSettings parameters(folder.absoluteFilePath("settings.ini"), QSettings::IniFormat);

    QString name = parameters.value("name").toString();
    QString code = parameters.value("code").toString();
    QString bbox = parameters.value("bbox").toString();
    QString projection = parameters.value("projection").toString();

    // This may leak
    GraphicProducer *gp = new GraphicProducer(this);
    connect(gp, SIGNAL(fillPath(QPainterPath,GraphicContext,Qt::FillRule)), this, SLOT(fillPath(QPainterPath,GraphicContext,Qt::FillRule)), Qt::QueuedConnection);
    connect(gp, SIGNAL(strikePath(QPainterPath,GraphicContext)), this, SLOT(strikePath(QPainterPath,GraphicContext)), Qt::QueuedConnection);
    connect(gp, SIGNAL(parsingDone(bool)), this, SLOT(documentParsed(bool)));
    connect(gp, SIGNAL(parsingDone(bool)), gp, SLOT(deleteLater()));

    // Hack to launch in a new thread ?
    QFuture<bool> result = QtConcurrent::run(gp, &GraphicProducer::parsePDF, pdfFileName);
}

void CadastreFranceVectorAdapter::resultsAvailable(QMap<QString, QString> results)
{
    if (results.size() > 1) {
        CadastreWrapper::instance()->searchCode(m_code, m_department);
        return;
    }

    disconnect(CadastreWrapper::instance(), SIGNAL(resultsAvailable(QMap<QString,QString>)), this, SLOT(resultsAvailable(QMap<QString,QString>)));
    m_city = CadastreWrapper::instance()->requestCity(m_code);
    updateMenu();
//    if (!CadastreWrapper::instance()->downloadTiles(m_city))
//        return;

    QDir dir = CadastreWrapper::instance()->getCacheDir();
    Q_ASSERT(dir.cd(m_city.code()));
    if (theImageManager)
        theImageManager->setCacheDir(dir);

    emit(forceProjection());
    emit(forceZoom());
    emit(forceRefresh());
}

bool CadastreFranceVectorAdapter::isValid(int x, int y, int z) const
{
    // Origin is bottom-left
    y = getTilesNS(current_zoom)-1 - y;

    if (m_city.code().isEmpty())
        return false;

    if ((x<0) || (x>=getTilesWE(z)) ||
            (y<0) || (y>=getTilesNS(z)))
    {
        return false;
    }
    return true;

}

QString CadastreFranceVectorAdapter::getQuery(int i, int j, int /* z */)  const
{
    qreal tileWidth = getBoundingbox().width() / getTilesWE(current_zoom);
    qreal tileHeight = getBoundingbox().height() / getTilesNS(current_zoom);

    QPointF ul = QPointF(i*tileWidth+getBoundingbox().topLeft().x(), getBoundingbox().bottomLeft().y()-j*tileHeight);
    QPointF br = QPointF((i+1)*tileWidth+getBoundingbox().topLeft().x(), getBoundingbox().bottomLeft().y()- (j+1)*tileHeight);

    QUrl theUrl("http://www.cadastre.gouv.fr/scpc/wms?version=1.1&request=GetMap&layers=CDIF:LS3,CDIF:LS2,CDIF:LS1,CDIF:PARCELLE,CDIF:NUMERO,CDIF:PT3,CDIF:PT2,CDIF:PT1,CDIF:LIEUDIT,CDIF:COMMUNE&format=image/png&exception=application/vnd.ogc.se_inimage&styles=LS3_90,LS2_90,LS1_90,PARCELLE_90,NUMERO_90,PT3_90,PT2_90,PT1_90,LIEUDIT_90,COMMUNE_90");
    theUrl.addQueryItem("WIDTH", QString::number(getTileSizeW()));
    theUrl.addQueryItem("HEIGHT", QString::number(getTileSizeH()));
    theUrl.addQueryItem("BBOX", QString()
                        .append(loc.toString(ul.x(),'f',6)).append(",")
                        .append(loc.toString(br.y(),'f',6)).append(",")
                        .append(loc.toString(br.x(),'f',6)).append(",")
                        .append(loc.toString(ul.y(),'f',6))
                        );

    return theUrl.toString(QUrl::RemoveScheme | QUrl::RemoveAuthority);
}

QString CadastreFranceVectorAdapter::getQuery(const QRectF& , const QRectF& projBbox, const QRect& size) const
{
    if (m_city.code().isEmpty())
        return QString();

    QUrl theUrl("http://www.cadastre.gouv.fr/scpc/wms?version=1.1&request=GetMap&layers=CDIF:LS3,CDIF:LS2,CDIF:LS1,CDIF:PARCELLE,CDIF:NUMERO,CDIF:PT3,CDIF:PT2,CDIF:PT1,CDIF:LIEUDIT,CDIF:COMMUNE&format=image/png&exception=application/vnd.ogc.se_inimage&styles=LS3_90,LS2_90,LS1_90,PARCELLE_90,NUMERO_90,PT3_90,PT2_90,PT1_90,LIEUDIT_90,COMMUNE_90");
    theUrl.addQueryItem("WIDTH", QString::number(size.width()));
    theUrl.addQueryItem("HEIGHT", QString::number(size.height()));
    theUrl.addQueryItem("BBOX", QString()
                        .append(loc.toString(projBbox.bottomLeft().x(),'f',6)).append(",")
                        .append(loc.toString(projBbox.bottomLeft().y(),'f',6)).append(",")
                        .append(loc.toString(projBbox.topRight().x(),'f',6)).append(",")
                        .append(loc.toString(projBbox.topRight().y(),'f',6))
                        );

    return theUrl.toString(QUrl::RemoveScheme | QUrl::RemoveAuthority);
}

QPixmap CadastreFranceVectorAdapter::getPixmap(const QRectF& /*wgs84Bbox*/, const QRectF& projBbox, const QRect& src) const
{
    QPixmap pix(src.size());
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.scale(src.width()/projBbox.width(), src.height()/projBbox.height());
    p.translate(-projBbox.left(), -projBbox.bottom());

    if (!m_city.code().isEmpty()) {
        QDir dir = CadastreWrapper::instance()->getCacheDir();
        Q_ASSERT(dir.cd(m_city.code()));

        for (int r=0; r<m_city.tileRows(); ++r) {
            for (int c=0; c<m_city.tileColumns(); ++c) {
                QRectF g = QRectF(m_city.tileGeometry(r, c));
                QRectF inter = g.intersected(projBbox);
                if (!inter.isNull()) {
                    QImage img(dir.absoluteFilePath(QString("%1-%2.png").arg(r).arg(c)));
                    p.drawImage(g.topLeft(), img);
                }
            }
        }
    }

    p.end();
    return pix;
}

void CadastreFranceVectorAdapter::cleanup()
{
}

bool CadastreFranceVectorAdapter::toXML(QDomElement xParent)
{
    Q_UNUSED(xParent)

    return true;
}

void CadastreFranceVectorAdapter::fromXML(const QDomElement xParent)
{
    Q_UNUSED(xParent)
}

QString CadastreFranceVectorAdapter::toPropertiesHtml()
{
    return "";
}

bool CadastreFranceVectorAdapter::isTiled() const { return true; }

int CadastreFranceVectorAdapter::getTilesWE(int zoomlevel) const
{
    qreal unitPerTile = Resolutions[zoomlevel] * getTileSizeW(); // Size of 1 tile in projected units
    return qRound(getBoundingbox().width() / unitPerTile);
}

int CadastreFranceVectorAdapter::getTilesNS(int zoomlevel) const
{
    qreal unitPerTile = Resolutions[zoomlevel] * getTileSizeH(); // Size of 1 tile in projected units
    return qRound(getBoundingbox().height() / unitPerTile);
}

int	CadastreFranceVectorAdapter::getTileSizeW	() const
{
    return 256;
}

int	CadastreFranceVectorAdapter::getTileSizeH	() const
{
    return 256;
}

void CadastreFranceVectorAdapter::zoom_in()
{
    current_zoom = current_zoom < max_zoom ? current_zoom+1 : max_zoom;

}
void CadastreFranceVectorAdapter::zoom_out()
{
    current_zoom = current_zoom > min_zoom ? current_zoom-1 : min_zoom;
}

int CadastreFranceVectorAdapter::getMinZoom() const
{
    return min_zoom;
}

int CadastreFranceVectorAdapter::getMaxZoom() const
{
    return max_zoom;
}

int CadastreFranceVectorAdapter::getAdaptedMinZoom() const
{
    return 0;
}

int CadastreFranceVectorAdapter::getAdaptedMaxZoom() const
{
    return max_zoom > min_zoom ? max_zoom - min_zoom : min_zoom - max_zoom;
}

int CadastreFranceVectorAdapter::getZoom() const
{
    return current_zoom;
}

int CadastreFranceVectorAdapter::getAdaptedZoom() const
{
    return max_zoom < min_zoom ? min_zoom - current_zoom : current_zoom - min_zoom;
}

Q_EXPORT_PLUGIN2(MCadastreFranceVectorBackgroundPlugin, CadastreFranceVectorAdapterFactory)

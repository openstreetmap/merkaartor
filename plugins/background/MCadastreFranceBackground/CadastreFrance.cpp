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
#include <QMessageBox>

#include "CadastreFrance.h"

#include "cadastrewrapper.h"
#include "searchdialog.h"

#include "IImageManager.h"

static const QUuid theUid ( 0x14a9ff26, 0x634e, 0x4406, 0x94, 0xa5, 0x4c, 0x6d, 0x9c, 0xf0, 0xb1, 0x1d);
static const QString theName("Cadastre (France)");

QUuid CadastreFranceAdapterFactory::getId() const
{
    return theUid;
}

QString	CadastreFranceAdapterFactory::getName() const
{
    return theName;
}

/**************/

CadastreFranceAdapter::CadastreFranceAdapter()
    : theImageManager(0), theMenu(0), theSettings(0)
    , current_zoom(0), min_zoom(0), max_zoom(6)
{
    loc = QLocale(QLocale::English);
    loc.setNumberOptions(QLocale::OmitGroupSeparator);

    Resolutions << 16 << 8. << 4. << 2 << 1.0 << 0.5 << 0.2;

    m_isTiled = true;
}

CadastreFranceAdapter::~CadastreFranceAdapter()
{
}

QUuid CadastreFranceAdapter::getId() const
{
    return theUid;
}

QString	CadastreFranceAdapter::getName() const
{
    return theName;
}

void CadastreFranceAdapter::setSettings(QSettings* aSet)
{
    theSettings = aSet;
    CadastreWrapper::instance()->setRootCacheDir(QDir(theSettings->value("backgroundImage/CacheDir").toString()));
    updateMenu();
}

QString	CadastreFranceAdapter::getHost() const
{
    return "www.cadastre.gouv.fr";
}

IMapAdapter::Type CadastreFranceAdapter::getType() const
{
    return IMapAdapter::NetworkBackground;
}

QString CadastreFranceAdapter::projection() const
{
    return m_city.projection();
}

QRectF CadastreFranceAdapter::getBoundingbox() const
{
    double L = qMax(m_city.geometry().width(), m_city.geometry().height());
    QRectF bb(m_city.geometry());
    QRectF R = QRectF(QPointF(bb.center().x()-L/2, bb.center().y()-L/2),
                      QPointF(bb.center().x()+L/2, bb.center().y()+L/2));
    return R;
//    return QRectF(R.bottomLeft(), R.topRight());
}

QMenu* CadastreFranceAdapter::getMenu() const
{
    return theMenu;
}

IImageManager* CadastreFranceAdapter::getImageManager()
{
    return theImageManager;
}

void CadastreFranceAdapter::setImageManager(IImageManager* anImageManager)
{
    theImageManager = anImageManager;
    CadastreWrapper::instance()->setNetworkManager(theImageManager->getNetworkManager());
    theImageManager->setCachePermanent(true);
}

void CadastreFranceAdapter::updateMenu()
{
    delete theMenu;
    theMenu = new QMenu(0);

    QAction* grabCity = new QAction(tr("Grab City..."), this);
    connect(grabCity, SIGNAL(triggered()), SLOT(onGrabCity()));
    theMenu->addAction(grabCity);
    QAction* tiledToggle = new QAction(tr("Tiled"), this);
    tiledToggle->setCheckable(true);
    tiledToggle->setChecked(m_isTiled);
    connect(tiledToggle, SIGNAL(triggered()), SLOT(toggleTiled()));
    theMenu->addAction(tiledToggle);

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

void CadastreFranceAdapter::toggleTiled()
{
    m_isTiled = !m_isTiled;
    updateMenu();
    emit(forceRefresh());
}

void CadastreFranceAdapter::onGrabCity()
{
    if (!theImageManager)
        return;
    m_city = City();

    SearchDialog *dial = new SearchDialog();
    dial->cadastre->setRootCacheDir(QDir(theSettings->value("backgroundImage/CacheDir").toString()));
    dial->setModal(true);
    if (dial->exec()) {
        m_code = dial->cityCode();
        QString name = dial->cityName();
        if (!name.isEmpty())
            initializeCity(name);
    }
    delete dial;
}

void CadastreFranceAdapter::cityTriggered(QAction *act)
{
    QString name = act->text();
    if (act->data().toString().isEmpty())
        return;
    m_code = act->data().toString();
    if (!theImageManager)
        return;
    m_city = City();
    initializeCity(name);
}

void CadastreFranceAdapter::initializeCity(QString name)
{
    qDebug() << "Initializing " << name;
    connect(CadastreWrapper::instance(), SIGNAL(resultsAvailable(QMap<QString,QString>)), this, SLOT(resultsAvailable(QMap<QString,QString>)));
    QString ville = name.left(name.lastIndexOf('(')-1);
    m_department = QString("%1").arg(name.mid(name.lastIndexOf('(')+1, name.lastIndexOf(')')-name.lastIndexOf('(')-1).toInt(), 3, 10, QChar('0'));
    CadastreWrapper::instance()->searchVille(ville, m_department);
}

void CadastreFranceAdapter::resultsAvailable(QMap<QString, QString> results)
{
    if (results.size() > 1) {
        CadastreWrapper::instance()->searchCode(m_code, m_department);
        return;
    }

    disconnect(CadastreWrapper::instance(), SIGNAL(resultsAvailable(QMap<QString,QString>)), this, SLOT(resultsAvailable(QMap<QString,QString>)));

    if (!results.size()) {
        QMessageBox::critical(0, tr("The city cannot be loaded"), tr("Only vectorized cities can be handled by this plugin and the selected one is still in \"Image\" format."));
        return;
    }

    m_city = CadastreWrapper::instance()->requestCity(m_code);
    updateMenu();
//    if (!CadastreWrapper::instance()->downloadTiles(m_city))
//        return;

    QDir dir = CadastreWrapper::instance()->getCacheDir();
    Q_ASSERT(dir.cd(m_city.code()));
    if (theImageManager)
        theImageManager->setCacheDir(dir);

    emit(forceZoom());
    emit(forceProjection());
    emit(forceRefresh());
}

bool CadastreFranceAdapter::isValid(int x, int y, int z) const
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

QString CadastreFranceAdapter::getQuery(int i, int j, int /* z */)  const
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

QString CadastreFranceAdapter::getQuery(const QRectF& , const QRectF& projBbox, const QRect& size) const
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

QPixmap CadastreFranceAdapter::getPixmap(const QRectF& /*wgs84Bbox*/, const QRectF& projBbox, const QRect& src) const
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

void CadastreFranceAdapter::cleanup()
{
}

bool CadastreFranceAdapter::toXML(QXmlStreamWriter& stream)
{
    Q_UNUSED(stream)

    return true;
}

void CadastreFranceAdapter::fromXML(const QDomElement xParent)
{
    Q_UNUSED(xParent)
}

QString CadastreFranceAdapter::toPropertiesHtml()
{
    return "";
}

bool CadastreFranceAdapter::isTiled() const
{
    return m_isTiled;
}

int CadastreFranceAdapter::getTilesWE(int zoomlevel) const
{
    qreal unitPerTile = Resolutions[zoomlevel] * getTileSizeW(); // Size of 1 tile in projected units
    return qRound(getBoundingbox().width() / unitPerTile);
}

int CadastreFranceAdapter::getTilesNS(int zoomlevel) const
{
    qreal unitPerTile = Resolutions[zoomlevel] * getTileSizeH(); // Size of 1 tile in projected units
    return qRound(getBoundingbox().height() / unitPerTile);
}

int	CadastreFranceAdapter::getTileSizeW	() const
{
    return 256;
}

int	CadastreFranceAdapter::getTileSizeH	() const
{
    return 256;
}

void CadastreFranceAdapter::zoom_in()
{
    current_zoom = current_zoom < max_zoom ? current_zoom+1 : max_zoom;

}
void CadastreFranceAdapter::zoom_out()
{
    current_zoom = current_zoom > min_zoom ? current_zoom-1 : min_zoom;
}

int CadastreFranceAdapter::getMinZoom(const QRectF &) const
{
    return min_zoom;
}

int CadastreFranceAdapter::getMaxZoom(const QRectF &) const
{
    return max_zoom;
}

int CadastreFranceAdapter::getAdaptedMinZoom(const QRectF &) const
{
    return 0;
}

int CadastreFranceAdapter::getAdaptedMaxZoom(const QRectF &) const
{
    return max_zoom > min_zoom ? max_zoom - min_zoom : min_zoom - max_zoom;
}

int CadastreFranceAdapter::getZoom() const
{
    return current_zoom;
}

int CadastreFranceAdapter::getAdaptedZoom() const
{
    return max_zoom < min_zoom ? min_zoom - current_zoom : current_zoom - min_zoom;
}

QString	CadastreFranceAdapter::getSourceTag		() const
{
    return "cadastre-dgi-fr source : Direction Générale des Impôts - Cadastre. Mise à jour : 2010";
}

Q_EXPORT_PLUGIN2(MCadastreFranceBackgroundPlugin, CadastreFranceAdapterFactory)

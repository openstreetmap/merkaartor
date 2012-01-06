#include "Global.h"

#include "LayerWidget.h"
#include "LayerDock.h"

#include "MainWindow.h"
#include "Document.h"
#include "Layer.h"
#include "MerkaartorPreferences.h"

#include "IMapAdapterFactory.h"
#include "IMapAdapter.h"

#include <QApplication>
#include <QMouseEvent>
#include <QStylePainter>
#include <QInputDialog>
#include <QMessageBox>

#include "SelectionDialog.h"

#define LINEHEIGHT 25

LayerWidget::LayerWidget(Layer* aLayer, QWidget* aParent)
: QPushButton(aParent), theLayer(aLayer), ctxMenu(0), closeAction(0), actZoom(0), associatedMenu(0)
{
    ui.setupUi(this);

    setCheckable(true);
#ifdef Q_OS_MAC
    setFlat(true);
#endif
    setFocusPolicy(Qt::NoFocus);
    setContextMenuPolicy(Qt::NoContextMenu);

    ui.cbVisible->blockSignals(true);
    ui.cbVisible->setChecked(theLayer->isVisible());
    ui.cbVisible->blockSignals(false);
#ifdef Q_OS_MAC
        ui.cbVisible->setMinimumWidth(30);
#endif

    ui.edName->setText(theLayer->name());
    ui.edName->setReadOnly(true);
    ui.edName->setAttribute(Qt::WA_TransparentForMouseEvents);

    ui.edName->setFrame(false);
    ui.edName->setCursorPosition(0);
    ui.edName->setStyleSheet(" background: transparent; ");

    connect(this, SIGNAL(toggled(bool)), SLOT(checkedStatusToggled(bool)));
}

LayerWidget::~LayerWidget()
{
    delete associatedMenu;
}

void LayerWidget::paintEvent(QPaintEvent *)
{
    QStylePainter p(this);
    QStyleOptionButton option;
    initStyleOption(&option);
    p.drawControl(QStyle::CE_PushButton, option);

    if (theLayer && !theLayer->isUploadable()) {
        p.fillRect(rect().adjusted(20,0,0,-1),QBrush(Qt::red, Qt::BDiagPattern));
    }

}

void LayerWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        dragStartPosition = event->pos();

    event->ignore();
}

void LayerWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;
    if ((event->pos() - dragStartPosition).manhattanLength()
        < QApplication::startDragDistance())
        return;

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/x-layer", theLayer->id().toLatin1());
    drag->setMimeData(mimeData);

    QPixmap px(size());
    render(&px);
    drag->setPixmap(px);

    /*Qt::DropAction dropAction =*/ drag->exec(Qt::MoveAction);
}

void LayerWidget::mouseReleaseEvent(QMouseEvent* anEvent)
{
    anEvent->ignore();
}

void LayerWidget::on_cbVisible_stateChanged ( int state )
{
    setLayerVisible((state > Qt::Unchecked));
}

void LayerWidget::mouseDoubleClickEvent(QMouseEvent */*event*/)
{
    ui.edName->setReadOnly(false);
    ui.edName->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    ui.edName->setFrame(true);
    ui.edName->setStyleSheet("");
    ui.edName->setFocus();
}

void LayerWidget::on_edName_editingFinished()
{
    ui.edName->setReadOnly(true);
    ui.edName->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui.edName->setFrame(false);
    ui.edName->setStyleSheet(" background: transparent; ");

    if (!ui.edName->text().isEmpty())
        theLayer->setName(ui.edName->text());
    else
        ui.edName->setText(theLayer->name());
}

void LayerWidget::checkedStatusToggled(bool newVal)
{
    theLayer->setSelected(newVal);
}

void LayerWidget::setName(const QString& s)
{
    ui.edName->setText(s);
}

Layer* LayerWidget::getLayer()
{
    return theLayer;
}

void LayerWidget::showContextMenu(QContextMenuEvent* anEvent)
{
    //initActions();

    if (!ctxMenu)
        return;

    if (actZoom) {
        actZoom->setEnabled(theLayer->size());
    }
    if (closeAction)
        closeAction->setEnabled(theLayer->canDelete());
    ctxMenu->exec(anEvent->globalPos());
}

#define NUMOP 3
void LayerWidget::initActions()
{
    SAFE_DELETE(ctxMenu);
    SAFE_DELETE(associatedMenu);

    ctxMenu = new QMenu(this);
    associatedMenu = new QMenu(theLayer->name());
    //connect(associatedMenu, SIGNAL(aboutToShow()), this, SLOT(associatedAboutToShow()));

    actVisible = new QAction(tr("Visible"), ctxMenu);
    actVisible->setCheckable(true);
    actVisible->setChecked(theLayer->isVisible());
    associatedMenu->addAction(actVisible);
    connect(actVisible, SIGNAL(triggered(bool)), this, SLOT(visibleLayer(bool)));

    actReadonly = new QAction(tr("Readonly"), ctxMenu);
    actReadonly->setCheckable(true);
    actReadonly->setChecked(theLayer->isReadonly());
    associatedMenu->addAction(actReadonly);
    ctxMenu->addAction(actReadonly);
    connect(actReadonly, SIGNAL(triggered(bool)), this, SLOT(readonlyLayer(bool)));

    static const char *opStr[NUMOP] = {
    QT_TR_NOOP("Low"), QT_TR_NOOP("High"), QT_TR_NOOP("Opaque")};

    QActionGroup* actgrp = new QActionGroup(this);
    QMenu* alphaMenu = new QMenu(tr("Opacity"), this);
    for (int i=0; i<NUMOP; i++) {
        QAction* act = new QAction(tr(opStr[i]), alphaMenu);
        actgrp->addAction(act);
        qreal a = M_PREFS->getAlpha(opStr[i]);
        act->setData(a);
        act->setCheckable(true);
        if (int(theLayer->getAlpha()*100) == int(a*100))
            act->setChecked(true);
        alphaMenu->addAction(act);
    }
    ctxMenu->addMenu(alphaMenu);
    connect(alphaMenu, SIGNAL(triggered(QAction*)), this, SLOT(setOpacity(QAction*)));
    associatedMenu->addMenu(alphaMenu);
}

void LayerWidget::setOpacity(QAction *act)
{
    theLayer->setAlpha(act->data().toDouble());
    emit (layerChanged(this, false));
}

void LayerWidget::close()
{
    if (theLayer.data()->getDirtyLevel()) {
        if (QMessageBox::question(this, tr("Layer CLose: Dirty objects present"),
                                     tr("There are dirty features on this layer.\n"
                                        "Are you sure you want to close it? (no Undo possible)"),
                                     QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Cancel)
            return;
    } else if (theLayer.data()->size())
        if (QMessageBox::question(this, tr("Layer CLose: Not empty"),
                                     tr("Are you sure you want to close this layer? (no Undo possible)"),
                                     QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Cancel)
            return;

    emit(layerClosed(theLayer));
}

void LayerWidget::clear()
{
    emit(layerCleared(theLayer));
}

void LayerWidget::zoomLayer()
{
    emit (layerZoom(theLayer));
}

void LayerWidget::visibleLayer(bool)
{
    setLayerVisible(actVisible->isChecked());
}

void LayerWidget::readonlyLayer(bool)
{
    setLayerReadonly(actReadonly->isChecked());
}

QMenu* LayerWidget::getAssociatedMenu()
{
    return associatedMenu;
}

void LayerWidget::setLayerVisible(bool b, bool updateLayer)
{
    actVisible->blockSignals(true);
    actVisible->setChecked(b);
    actVisible->blockSignals(false);

    ui.cbVisible->blockSignals(true);
    ui.cbVisible->setChecked(b);
    ui.cbVisible->blockSignals(false);

    update();

    if (updateLayer) {
        theLayer->setVisible(b);
        emit(layerChanged(this, false));
    }
}

void LayerWidget::setLayerReadonly(bool b)
{
    theLayer->setReadonly(b);
    actReadonly->setChecked(b);
    update();
    emit(layerChanged(this, false));
}

void LayerWidget::associatedAboutToShow()
{
    initActions();
}


// DrawingLayerWidget

DrawingLayerWidget::DrawingLayerWidget(DrawingLayer* aLayer, QWidget* aParent)
    : LayerWidget(aLayer, aParent)
{
    initActions();
}

void DrawingLayerWidget::initActions()
{
    LayerWidget::initActions();

    ctxMenu->addSeparator();
    associatedMenu->addSeparator();

    actZoom = new QAction(tr("Zoom"), ctxMenu);
    ctxMenu->addAction(actZoom);
    associatedMenu->addAction(actZoom);
    connect(actZoom, SIGNAL(triggered(bool)), this, SLOT(zoomLayer()));

    closeAction = new QAction(tr("Close"), this);
    connect(closeAction, SIGNAL(triggered()), this, SLOT(close()));
    ctxMenu->addAction(closeAction);
    associatedMenu->addAction(closeAction);
    closeAction->setEnabled(theLayer->canDelete());
}

// ImageLayerWidget

ImageLayerWidget::ImageLayerWidget(ImageMapLayer* aLayer, QWidget* aParent)
: LayerWidget(aLayer, aParent), wmsMenu(0) //, actgrWms(0)
{
    actNone = new QAction(tr("None"), this);
    //actNone->setCheckable(true);
    actNone->setChecked((M_PREFS->getBackgroundPlugin() == NONE_ADAPTER_UUID));
    actNone->setData(QVariant::fromValue(NONE_ADAPTER_UUID));
    connect(actNone, SIGNAL(triggered()), this, SLOT(setNone()));

    if (M_PREFS->getUseShapefileForBackground()) {
        actShape = new QAction(tr("Shape adapter"), this);
        //actShape->setCheckable(true);
        actShape->setChecked((M_PREFS->getBackgroundPlugin() == SHAPE_ADAPTER_UUID));
        actShape->setData(QVariant::fromValue(SHAPE_ADAPTER_UUID));
    }

    initActions();
}

ImageLayerWidget::~ImageLayerWidget()
{
}

void ImageLayerWidget::setNone()
{
    ((ImageMapLayer *)theLayer.data())->setMapAdapter(NONE_ADAPTER_UUID);

    this->update(rect());
    emit (layerChanged(this, true));
}

void ImageLayerWidget::setWms(QAction* act)
{
    WmsServerList* L = M_PREFS->getWmsServers();
    WmsServer S = L->value(act->data().toString());
    M_PREFS->setSelectedServer(S.WmsName);

    ((ImageMapLayer *)theLayer.data())->setMapAdapter(WMS_ADAPTER_UUID, S.WmsName);
    theLayer->setVisible(true);

    this->update(rect());
    emit (layerChanged(this, true));
}

void ImageLayerWidget::setTms(QAction* act)
{
    TmsServerList* L = M_PREFS->getTmsServers();
    TmsServer S = L->value(act->data().toString());
    M_PREFS->setSelectedServer(S.TmsName);

    ((ImageMapLayer *)theLayer.data())->setMapAdapter(TMS_ADAPTER_UUID, S.TmsName);
    theLayer->setVisible(true);

    this->update(rect());
    emit (layerChanged(this, true));
}

void ImageLayerWidget::setPlugin(QAction* act)
{
    QUuid aUuid = act->data().value<QUuid>();
    if (aUuid.isNull())
        return;

    ((ImageMapLayer *)theLayer.data())->setMapAdapter(aUuid);
    theLayer->setVisible(true);

    this->update(rect());
    emit (layerChanged(this, true));
}

void ImageLayerWidget::setProjection()
{
    IMapAdapter* ma = ((ImageMapLayer*)theLayer.data())->getMapAdapter();
    if (ma) {
        emit (layerProjection(ma->projection()));
    }
}

void ImageLayerWidget::resetAlign()
{
    ((ImageMapLayer *)theLayer.data())->resetAlign();
    emit (layerChanged(this, true));
}

void ImageLayerWidget::initActions()
{
    //if (actgrWms)
    //	delete actgrWms;
    //actgrWms = new QActionGroup(this);

    LayerWidget::initActions();
//    ImageMapLayer* il = ((ImageMapLayer *)theLayer.data());

    actZoom = new QAction(tr("Zoom"), ctxMenu);
    ctxMenu->addAction(actZoom);
    associatedMenu->addAction(actZoom);
    connect(actZoom, SIGNAL(triggered(bool)), this, SLOT(zoomLayer()));

    actReadonly->setVisible(false);

    actProjection = new QAction(tr("Set view projection to layer's"), this);
    connect(actProjection, SIGNAL(triggered()), this, SLOT(setProjection()));
    ctxMenu->addAction(actProjection);
    associatedMenu->addAction(actProjection);

    actResetAlign = new QAction(tr("Reset alignment adjustment"), this);
    connect(actResetAlign, SIGNAL(triggered()), this, SLOT(resetAlign()));
    ctxMenu->addAction(actResetAlign);
    associatedMenu->addAction(actResetAlign);

    closeAction = new QAction(tr("Close"), this);
    connect(closeAction, SIGNAL(triggered()), this, SLOT(close()));
    ctxMenu->addAction(closeAction);
    associatedMenu->addAction(closeAction);

    ctxMenu->addSeparator();
    associatedMenu->addSeparator();

    wmsMenu = new QMenu(tr("WMS adapter"), this);
    WmsServerList* WmsServers = M_PREFS->getWmsServers();
    WmsServerListIterator wi(*WmsServers);
    while (wi.hasNext()) {
        wi.next();
        WmsServer S = wi.value();
        if (!S.deleted) {
            QAction* act = new QAction(S.WmsName, wmsMenu);
            act->setData(S.WmsName);
            wmsMenu->addAction(act);
            if (M_PREFS->getBackgroundPlugin() == WMS_ADAPTER_UUID)
                if (S.WmsName == M_PREFS->getSelectedServer())
                    act->setChecked(true);
        }
    }

    tmsMenu = new QMenu(tr("TMS adapter"), this);
    TmsServerList* TmsServers = M_PREFS->getTmsServers();
    TmsServerListIterator ti(*TmsServers);
    while (ti.hasNext()) {
        ti.next();
        TmsServer S = ti.value();
        if (!S.deleted) {
            QAction* act = new QAction(S.TmsName, tmsMenu);
            act->setData(S.TmsName);
            tmsMenu->addAction(act);
            if (M_PREFS->getBackgroundPlugin() == TMS_ADAPTER_UUID)
                if (S.TmsName == M_PREFS->getSelectedServer())
                    act->setChecked(true);
        }
    }

    pluginsMenu = new QMenu(tr("Plugins"), this);
    QMapIterator <QUuid, IMapAdapterFactory *> it(M_PREFS->getBackgroundPlugins());
    QMap<QString, QUuid> plugins;
    while (it.hasNext()) {
        it.next();

        plugins[it.value()->getName()] = it.key();
    }
    QMapIterator <QString, QUuid> pluginsIt(plugins);
    while (pluginsIt.hasNext()) {
        pluginsIt.next();

        QAction* actBackPlug = new QAction(pluginsIt.key(), this);
        actBackPlug->setChecked((M_PREFS->getBackgroundPlugin() == pluginsIt.value()));
        actBackPlug->setData(QVariant::fromValue(pluginsIt.value()));

        pluginsMenu->addAction(actBackPlug);
    }

    actNone->setChecked((M_PREFS->getBackgroundPlugin() == NONE_ADAPTER_UUID));
    if (M_PREFS->getUseShapefileForBackground())
        actShape->setChecked((M_PREFS->getBackgroundPlugin() == SHAPE_ADAPTER_UUID));

    ctxMenu->addAction(actNone);
    associatedMenu->addAction(actNone);

    ctxMenu->addMenu(wmsMenu);
    associatedMenu->addMenu(wmsMenu);
    connect(wmsMenu, SIGNAL(triggered(QAction*)), this, SLOT(setWms(QAction*)));

    ctxMenu->addMenu(tmsMenu);
    associatedMenu->addMenu(tmsMenu);
    connect(tmsMenu, SIGNAL(triggered(QAction*)), this, SLOT(setTms(QAction*)));

    ctxMenu->addMenu(pluginsMenu);
    associatedMenu->addMenu(pluginsMenu);
    connect(pluginsMenu, SIGNAL(triggered(QAction*)), this, SLOT(setPlugin(QAction*)));

    if (M_PREFS->getUseShapefileForBackground()) {
        ctxMenu->addAction(actShape);
        associatedMenu->addAction(actShape);
    }
}

void ImageLayerWidget::showContextMenu(QContextMenuEvent* anEvent)
{
    if (!ctxMenu)
        return;

    ImageMapLayer* theMapLayer = qobject_cast<ImageMapLayer*>(theLayer);
    QList<QAction*> plugActions;

    QAction* sep = new QAction(this);
    sep->setSeparator(true);

    actZoom->setEnabled(false);
    if (theMapLayer && theMapLayer->getMapAdapter()) {
        if (!theMapLayer->getMapAdapter()->getBoundingbox().isNull())
            actZoom->setEnabled(true);

        if (theMapLayer->getMapAdapter()->getMenu()) {
            ctxMenu->addAction(sep);
            associatedMenu->addAction(sep);
            plugActions << sep;
            foreach (QAction* a, theMapLayer->getMapAdapter()->getMenu()->actions()) {
                ctxMenu->addAction(a);
                associatedMenu->addAction(a);
                plugActions << a;
            }
        }
    }
    closeAction->setEnabled(theLayer->canDelete());

    ctxMenu->exec(anEvent->globalPos());

    foreach (QAction* a, plugActions) {
        if (ctxMenu->actions().contains(a))
            ctxMenu->removeAction(a);
        if (associatedMenu->actions().contains(a))
            associatedMenu->removeAction(a);
    }
    delete sep;
}

// TrackLayerWidget

TrackLayerWidget::TrackLayerWidget(TrackLayer* aLayer, QWidget* aParent)
    : LayerWidget(aLayer, aParent)
{
    initActions();
}

void TrackLayerWidget::initActions()
{
    LayerWidget::initActions();
    ctxMenu->addSeparator();
    associatedMenu->addSeparator();

    QAction* actExtract = new QAction(tr("Extract Drawing layer"), ctxMenu);
    ctxMenu->addAction(actExtract);
    associatedMenu->addAction(actExtract);
    connect(actExtract, SIGNAL(triggered(bool)), this, SLOT(extractLayer(bool)));

    actZoom = new QAction(tr("Zoom"), ctxMenu);
    ctxMenu->addAction(actZoom);
    associatedMenu->addAction(actZoom);
    connect(actZoom, SIGNAL(triggered(bool)), this, SLOT(zoomLayer()));

    ctxMenu->addSeparator();
    associatedMenu->addSeparator();

    closeAction = new QAction(tr("Close"), this);
    connect(closeAction, SIGNAL(triggered()), this, SLOT(close()));
    ctxMenu->addAction(closeAction);
    associatedMenu->addAction(closeAction);
    closeAction->setEnabled(theLayer->canDelete());
}

TrackLayerWidget::~TrackLayerWidget()
{
}

void TrackLayerWidget::extractLayer(bool)
{
    ((TrackLayer*)theLayer.data())->extractLayer();
    emit (layerChanged(this, false));
}

// SpecialLayerWidget

SpecialLayerWidget::SpecialLayerWidget(SpecialLayer* aLayer, QWidget* aParent)
    : LayerWidget(aLayer, aParent)
{
    initActions();
}

void SpecialLayerWidget::initActions()
{
    LayerWidget::initActions();
    ctxMenu->addSeparator();
    associatedMenu->addSeparator();

    QAction* actExtract = new QAction(tr("Refresh layer"), ctxMenu);
    ctxMenu->addAction(actExtract);
    associatedMenu->addAction(actExtract);
    connect(actExtract, SIGNAL(triggered(bool)), this, SLOT(refreshLayer(bool)));

    closeAction = new QAction(tr("Close"), this);
    connect(closeAction, SIGNAL(triggered()), this, SLOT(close()));
    ctxMenu->addAction(closeAction);
    associatedMenu->addAction(closeAction);
    closeAction->setEnabled(theLayer->canDelete());
}

SpecialLayerWidget::~SpecialLayerWidget()
{
}

void SpecialLayerWidget::refreshLayer(bool)
{
    dynamic_cast<SpecialLayer*>(theLayer.data())->refreshLayer();
}

// DirtyLayerWidget

DirtyLayerWidget::DirtyLayerWidget(DirtyLayer* aLayer, QWidget* aParent)
    : LayerWidget(aLayer, aParent)
{
    initActions();
}

void DirtyLayerWidget::initActions()
{
    LayerWidget::initActions();
    ctxMenu->addSeparator();
    associatedMenu->addSeparator();

    actZoom = new QAction(tr("Zoom"), ctxMenu);
    ctxMenu->addAction(actZoom);
    associatedMenu->addAction(actZoom);
    connect(actZoom, SIGNAL(triggered(bool)), this, SLOT(zoomLayer()));
}

// UploadLayerWidget

UploadedLayerWidget::UploadedLayerWidget(UploadedLayer* aLayer, QWidget* aParent)
    : LayerWidget(aLayer, aParent)
{
    initActions();
}

void UploadedLayerWidget::initActions()
{
    LayerWidget::initActions();
    ctxMenu->addSeparator();
    associatedMenu->addSeparator();

    actZoom = new QAction(tr("Zoom"), ctxMenu);
    ctxMenu->addAction(actZoom);
    associatedMenu->addAction(actZoom);
    connect(actZoom, SIGNAL(triggered(bool)), this, SLOT(zoomLayer()));

    closeAction = new QAction(tr("Clear"), this);
    connect(closeAction, SIGNAL(triggered()), this, SLOT(clear()));
    ctxMenu->addAction(closeAction);
    associatedMenu->addAction(closeAction);
    closeAction->setEnabled(theLayer->canDelete());
}

// FilterLayerWidget

FilterLayerWidget::FilterLayerWidget(FilterLayer* aLayer, QWidget* aParent)
    : LayerWidget(aLayer, aParent)
{
    initActions();
}

void FilterLayerWidget::initActions()
{
    LayerWidget::initActions();

    ctxMenu->addSeparator();
    associatedMenu->addSeparator();

    closeAction = new QAction(tr("Close"), this);
    connect(closeAction, SIGNAL(triggered()), this, SLOT(close()));
    ctxMenu->addAction(closeAction);
    associatedMenu->addAction(closeAction);
    closeAction->setEnabled(theLayer->canDelete());
}

void FilterLayerWidget::mouseDoubleClickEvent(QMouseEvent */*event*/)
{
    FilterLayer* Fl = dynamic_cast<FilterLayer*>(theLayer.data());

    QDialog* dlg = new QDialog(this);
    ui = new Ui::FilterEditDialog;
    ui->setupUi(dlg);
    ui->edName->setText(Fl->name());
    ui->edFilter->setText(Fl->filter());
    connect(ui->btFilterHelper, SIGNAL(clicked()), SLOT(on_filterHelperClicked()));
    if (dlg->exec() == QDialog::Accepted) {
        setName(ui->edName->text());
        Fl->setName(ui->edName->text());
        Fl->setFilter(ui->edFilter->text());
    }
    delete ui;
}

void FilterLayerWidget::on_filterHelperClicked()
{
    SelectionDialog* Sel = new SelectionDialog(g_Merk_MainWindow, false);
    if (!Sel)
        return;

    Sel->edTagQuery->setText(ui->edFilter->text());
    if (Sel->exec() == QDialog::Accepted) {
        ui->edFilter->setText(Sel->edTagQuery->text());
    }
}


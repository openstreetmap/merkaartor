#include "GeoImageDock.h"

#include "Maps/TrackPoint.h"
#include "Maps/MapLayer.h"
#include "Command/DocumentCommands.h"
#include "LayerWidget.h"
#include "PropertiesDock.h"

#include <QtGui/QInputDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QProgressDialog>
#include <QtGui/QClipboard>
#include <QtGui/QRadioButton>
#include <QtGui/QTimeEdit>
#include <QtGui/QDialogButtonBox>


GeoImageDock::GeoImageDock(MainWindow *aMain)
	: QDockWidget(aMain), Main(aMain)
{
	curImage = -1;
	updateByMe = false;
	setWindowTitle(tr("Geo Images"));
	Image = new ImageView(this);
	setWidget(Image);
	setObjectName("geoImageDock");

	setContextMenuPolicy(Qt::ActionsContextMenu);

	QAction *remImages = new QAction(tr("Remove Images"), this);
	QAction *toClipboard = new QAction(tr("Copy filename to clipboard"), this);
	QAction *nextImage = new QAction(tr("Select next image"), Main); // make it available from everywhere
	nextImage->setShortcut(tr("PgDown"));
	QAction *previousImage = new QAction(tr("Select previous image"), Main); // make it available from everywhere
	previousImage->setShortcut(tr("PgUp"));

	addAction(remImages);
	addAction(toClipboard);
	addAction(nextImage);
	addAction(previousImage);

	connect(remImages, SIGNAL(triggered()), this, SLOT(removeImages()));
	connect(toClipboard, SIGNAL(triggered()), this, SLOT(toClipboard()));
	connect(nextImage, SIGNAL(triggered()), this, SLOT(selectNext()));
	connect(previousImage, SIGNAL(triggered()), this, SLOT(selectPrevious()));
}

GeoImageDock::~GeoImageDock(void)
{
	delete widget();
}

void GeoImageDock::setImage(TrackPoint *Pt)
{
	if (updateByMe)
		return;
	if (!Pt) {
		Image->setImage("");
		curImage = -1;
		return;
	}

	int ImageId;
	QString id = Pt->id();
	for (ImageId = 0; ImageId < usedTrackPoints.size(); ImageId++) // search for an entry in our list
		if (usedTrackPoints.at(ImageId).id == id)
			break;

	if (ImageId == curImage)
		return;

	if (ImageId == usedTrackPoints.size()) { // haven't found it
		Image->setImage("");
		curImage = -1;
		return;
	}

	Image->setImage(usedTrackPoints.at(ImageId).filename);
	curImage = ImageId;
}
void GeoImageDock::setImage(int ImageId)
{
	if (ImageId < 0 || ImageId >= usedTrackPoints.size()) { // invalid ImageId
		Image->setImage("");
		curImage = -1;
		return;
	}

	VisibleFeatureIterator it(Main->document());
	for (; !it.isEnd(); ++it) // find TrackPoint
		if (usedTrackPoints.at(ImageId).id == it.get()->id())
			break;

	if (it.isEnd()) { // haven't found one
		Image->setImage("");
		curImage = -1;
		return;
	}

	updateByMe = true;
	if (!Main->properties()->isSelected(it.get())) {
		Main->properties()->setSelection(it.get());
		Main->view()->invalidate(true, false);
	}
	updateByMe = false;

	Image->setImage(usedTrackPoints.at(ImageId).filename);
	curImage = ImageId;
}

void GeoImageDock::removeImages(void)
{
	int i;

	for (i = 0; i < usedTrackPoints.size(); i++) {
		TrackPoint *Pt = dynamic_cast<TrackPoint*>(Main->document()->getFeature(usedTrackPoints.at(i).id));
		if (!Pt) {
			qWarning("This should not happen. See %s::%d!", __FILE__, __LINE__);
			continue;
		}
		if (usedTrackPoints.at(i).inserted) {
			Pt->layer()->remove(Pt);
			delete Pt;
		}
		else
			Pt->clearTag("Picture");
	}

	usedTrackPoints.clear();
	curImage = -1;
	Image->setImage("");

	Main->view()->invalidate(true, false);
}
	
void GeoImageDock::toClipboard(void)
{
	if (curImage != -1) {
		QClipboard *clipboard = QApplication::clipboard();

		clipboard->setText(usedTrackPoints.at(curImage).filename);
	}
}

void GeoImageDock::selectNext(void)
{
	if (++curImage >= usedTrackPoints.size() || curImage < 0)
		curImage = 0;

	setImage(curImage);
}
void GeoImageDock::selectPrevious(void)
{
	if (--curImage < 0)
		curImage = usedTrackPoints.size() - 1;

	setImage(curImage);
}


void GeoImageDock::loadImages(QStringList fileNames)
{
	QString file;
	QDateTime time;
	int offset = -1, timeQuestion = 0, noMatchQuestion = 0;

	MapDocument *theDocument = Main->document();
	MapView *theView = Main->view();

	Exiv2::Image::AutoPtr image;
	Exiv2::ExifData exifData;
	double lat = 0.0, lon = 0.0;
	bool positionValid = FALSE;

	MapLayer *theLayer;
	{ // retrieve the target layer from the user
		QStringList layers;
		QList<int> layerId;
		int i;
		MapLayer *layer;
		MapLayer *singleLayer = NULL;
		MapLayer *singleTrackLayer = NULL;
		int trackLayersCount = 0;
		for (i=0;i<theDocument->layerSize();i++) {
			layer = theDocument->getLayer(i);
			if (layer->classType() == MapLayer::TrackMapLayerType) {
				trackLayersCount++;
				if (!singleTrackLayer)
					singleTrackLayer = layer;
			}
			if (layer->classType() == MapLayer::TrackMapLayerType || layer->classType() == MapLayer::DrawingMapLayerType) {
				if (!singleLayer)
					singleLayer = layer;
				layers.append(theDocument->getLayer(i)->name());
				layerId.append(i);
			}
		}

		if (layers.size() == 0) {
			QMessageBox::critical(this, tr("No layers"), tr("No suitable layer found. Please first download data from OSM server or open a track."));
			return;
		}

		// Select single layer if there is only one
		if (layers.size() == 1)
		{
			theLayer = singleLayer;
		}
		// Select single track layer if there is only one
		else if (trackLayersCount == 1)
		{
			theLayer = singleTrackLayer;
		}
		// Now ask the user what layer to add the photos to
		else
		{
			bool ok;
			QString name = QInputDialog::getItem(NULL, tr("Load geotagged Images"),
			 tr("Select the layer to which the images belong:"), layers, 0, false, &ok);
			if (ok && !name.isEmpty())
				theLayer = theDocument->getLayer(layerId.at(layers.indexOf(name)));
			else
				return;
		}
	}

	if (theLayer->isReadonly()) { // nodes from readonly layers can not be selected and therefore associated images can not be displayed
		if (QMessageBox::question(this, tr("Layer is readonly"),
		 tr("The used layer is not writeable. Should it be made writeable?\nIf not, you can't load images that belongs to it."),
		 QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes) == QMessageBox::Yes)
			theLayer->getWidget()->setLayerReadonly(false); // this makes/updates both the widget and the layer with readonly = false
		else
			return;
	}

	QProgressDialog progress(tr("Loading Images ..."), tr("Abort loading"), 0, fileNames.size());
	progress.setWindowModality(Qt::WindowModal);
	progress.show();

	foreach(file, fileNames) {
		progress.setValue(fileNames.indexOf(file));

		if (!QFile::exists(file))
			WARNING(tr("No such file"), tr("Can't find image \"%1\".").arg(file));

		try {
			image = Exiv2::ImageFactory::open(file.toStdString());
		}
		catch (Exiv2::Error error)
			WARNING(tr("Exiv2"), tr("Error while opening \"%2\":\n%1").arg(error.what()).arg(file));
		if (image.get() == 0)
			WARNING(tr("Exiv2"), tr("Error while loading EXIF-data from \"%1\".").arg(file));

		image->readMetadata();

		exifData = image->exifData();
		time = QDateTime();
		if (!exifData.empty()) {
			Exiv2::Exifdatum &latV = exifData["Exif.GPSInfo.GPSLatitude"];
			Exiv2::Exifdatum &lonV = exifData["Exif.GPSInfo.GPSLongitude"];
			positionValid = latV.count()==3 && lonV.count()==3;

			if (positionValid) {
				lat = latV.toFloat(0) + latV.toFloat(1) / 60.0 + latV.toFloat(2) / 3600.0;
				lon = lonV.toFloat(0) + lonV.toFloat(1) / 60.0 + lonV.toFloat(2) / 3600.0;
				if (exifData["Exif.GPSInfo.GPSLatitudeRef"].toString() == "S")
					lat *= -1.0;
				if (exifData["Exif.GPSInfo.GPSLongitudeRef"].toString() == "W")
					lon *= -1.0;
			}

			QString timeStamp = QString::fromStdString(exifData["Exif.Image.DateTime"].toString());
			if (timeStamp.isEmpty())
				timeStamp = QString::fromStdString(exifData["Exif.Photo.DateTimeOriginal"].toString());

			if (!timeStamp.isEmpty())
				time = QDateTime::fromString(timeStamp, "yyyy:MM:dd hh:mm:ss");
		}
		if (exifData.empty() || (!positionValid && time.isNull()) ) {
			// this question is asked when the file timestamp is used to find out to which node the image belongs
			QUESTION(tr("No EXIF"), tr("No EXIF header found in image \"%1\".\nDo you want to revert to improper file timestamp?").arg(file), timeQuestion);
			time = QFileInfo(file).created();
		}
		if (time.isNull()) // if time is still null, we use the file date as reference for image sorting (and not for finding out to which node the image belongs)
			// so we don't have to ask a question here
			time = QFileInfo(file).created();


		if (positionValid) {
			Coord newPos(angToInt(lat), angToInt(lon));
			TrackPoint *Pt;
            QList<MapFeature*>::ConstIterator it = theLayer->get().constBegin();
            QList<MapFeature*>::ConstIterator end = theLayer->get().constEnd();
			for (; it != end; it++) // use existing TrackPoint if there is one in small distance 
				if ((Pt = qobject_cast<TrackPoint*>(*it)) &&
				 Pt->position().distanceFrom(newPos) <= .002)
					break;
			if (it == end)
				Pt = new TrackPoint(newPos);

			Pt->setTag("Picture", "GeoTagged");
			usedTrackPoints << TrackPointData(Pt->id(), file, time, it == end);
			if (it == end)
				theLayer->add(Pt);
				//new AddFeatureCommand(theLayer, Pt, false);
		} else if (!time.isNull()) {
	
			if (offset == -1) { // ask the user to specify an offset for the images
				QDialog dialog(this);
				dialog.setWindowTitle(tr("Specify offset"));

				QLabel position(tr("Position images more to the:"), &dialog);
				QRadioButton positive(tr("end of the track"), &dialog);
				QRadioButton negative(tr("beginning of the track"), &dialog);
				QTimeEdit timeEdit(&dialog);
				QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);

				timeEdit.setDisplayFormat(tr("hh:mm:ss"));

				connect(&buttons, SIGNAL(accepted()), &dialog, SLOT(accept()));
				connect(&buttons, SIGNAL(rejected()), &dialog, SLOT(reject()));

				QVBoxLayout layout(&dialog); // very important to first declare the QVBoxLayout.
				QHBoxLayout radioLayout; // otherwise there would be a segmentation fault when return;
				QHBoxLayout timeLayout;

				radioLayout.addWidget(&positive);
				radioLayout.addWidget(&negative);
				timeLayout.addStretch();
				timeLayout.addWidget(&timeEdit); // center and make as small as possible
				timeLayout.addStretch();

				layout.addWidget(&position);
				layout.addLayout(&radioLayout);
				layout.addLayout(&timeLayout);
				layout.addWidget(&buttons);

				dialog.setLayout(&layout);

				if (dialog.exec()) { // we have to change the sign here because secsTo returns negative value
					if (positive.isChecked())
						offset = - timeEdit.time().secsTo(QTime(0, 0, 0));
					else if (negative.isChecked())
						offset = timeEdit.time().secsTo(QTime(0, 0, 0));
					else
						offset = 0;
				} else {
					theView->invalidate(true, false);
					return;
				}
			}

			time = time.addSecs(offset);

			MapFeature *feature = NULL;
			TrackPoint *Pt, *bestPt = NULL;
			int a, secondsTo = INT_MAX;
			int u;

			for (u=0; u<theLayer->size(); u++) {
				feature = theLayer->get(u);
				if ((Pt = dynamic_cast<TrackPoint*>(feature))) {
					a = time.secsTo(Pt->time().toLocalTime());
					if (abs(a) < abs(secondsTo)) {
						secondsTo = a;
						bestPt = Pt;
					}
				}
			}

			if (!bestPt)
				WARNING(tr("No TrackPoints"), tr("No TrackPoints found for image \"%1\""));

			if (abs(secondsTo) >= 15) {
				QTime difference = QTime().addSecs(abs(secondsTo));
				QString display;
				if (difference.hour() == 0)
					if (difference.minute() == 0)
						display = difference.toString(tr("ss 'seconds'"));
					else
						display = difference.toString(tr("mm 'minutes and' ss 'seconds'"));
				else
					display = difference.toString(tr("hh 'hours,' mm 'minutes and' ss 'seconds'"));
				QUESTION(tr("Wrong image?"), secondsTo > 0 ?
				 tr("Image \"%1\" was taken %2 before the next trackpoint was recorded.\nDo you still want to use it?").arg(file).arg(display) :
				 tr("Image \"%1\" was taken %2 after the last trackpoint was recorded.\nDo you still want to use it?").arg(file).arg(display),
				 noMatchQuestion);
			}

			usedTrackPoints << TrackPointData(bestPt->id(), file, time, false);
			bestPt->setTag("Picture", "GeoTagged");
	
			time = QDateTime(); // empty time to be null for the next image
		} else
			WARNING(tr("No geo informations"), tr("Image \"%1\" is not a geotagged image."));

		if (progress.wasCanceled()) {
			theView->invalidate(true, false);
			return;
		}
		qApp->processEvents();
	}

	progress.setValue(fileNames.size());

	qSort(usedTrackPoints); // sort them chronological
	curImage = -1; // the sorting invalidates curImage

	theView->invalidate(true, false);

}

void GeoImageDock::addGeoDataToImage(Coord position, const QString & file)
{
	Exiv2::Image::AutoPtr image;

	try {
		image = Exiv2::ImageFactory::open(file.toStdString());
	}
	catch (Exiv2::Error error) {
		QMessageBox::warning(this, tr("Exiv2"), tr("Error while opening \"%1\":\n%2").arg(file).arg(error.what()), QMessageBox::Ok);
		return;
	}
	if (image.get() == 0) {
		QMessageBox::warning(this, tr("Exiv2"), tr("Error while loading EXIF-data from \"%1\".").arg(file), QMessageBox::Ok);
		return;
	}

	image->readMetadata();

	double lat = intToAng(position.lat());
	double lon = intToAng(position.lon());

	QString hourFormat("%1/1 %2/1 %3/100");

	int h = lat / 1; // translate angle to hours, minutes and seconds
	int m = (lat - h) * 60 / 1;
	int s = (lat - h - m/60.0) * 60 * 60 * 100 / 1; // multiply with 100 because of divider in hourFormat
	Exiv2::ValueType<Exiv2::URational> vlat;
	vlat.read(hourFormat.arg(h).arg(m).arg(s).toStdString()); // fill vlat with string
	Exiv2::ExifKey klat("Exif.GPSInfo.GPSLatitude");
	Exiv2::ExifData::iterator pos;
	while ((pos = image->exifData().findKey(klat)) != image->exifData().end())
		image->exifData().erase(pos);
	image->exifData().add(klat, &vlat); // add key with value

	h = lon / 1; // translate angle to hours, minutes and seconds
	m = (lon - h) * 60 / 1;
	s = (lon - h - m/60.0) * 60 * 60 * 100 / 1; // multiply with 100 because of divider in hourFormat
	Exiv2::ValueType<Exiv2::URational> vlon;
	vlon.read(hourFormat.arg(h).arg(m).arg(s).toStdString()); // fil vlon with string
	Exiv2::ExifKey klon("Exif.GPSInfo.GPSLongitude");
	while ((pos = image->exifData().findKey(klon)) != image->exifData().end())
		image->exifData().erase(pos);
	image->exifData().add(klon, &vlon); // add key with value

	image->writeMetadata(); // store it

	loadImages(QStringList(file)); // loadImages now can load the data and display the image

	return;
}


// *** ImageView *** //

ImageView::ImageView(QWidget *parent)
	: QWidget(parent)
{
	zoomLevel = 1.0;
}

ImageView::~ImageView()
{
}

void ImageView::setImage(QString filename)
{
	name = filename;
	if (!name.isEmpty())
		image.load(name);
	else
		image = QImage();
	area = QRectF(QPoint(0, 0), image.size());
	zoomLevel = 1.0;
	resizeEvent(NULL);
	update();
}

void ImageView::paintEvent(QPaintEvent * /* e */)
{
	QPainter P(this);

	P.setRenderHint(QPainter::SmoothPixmapTransform);
	P.drawImage(rect, image, area, Qt::OrderedDither); // draw the image

	QRect text = QFontMetrics(P.font()).boundingRect(name); // calculate size of filename
	text.translate(-text.topLeft()); // move topLeft to (0, 0)
	if (text.width() > width())
		text.setWidth(width()); // max size is width()

	P.fillRect(text, QColor(255, 255, 255, 192)); // draw the text background

	if (text.width() == width()) { // draw a cutting text ("...") in front of the cutted filename
		QRect cutting = QFontMetrics(P.font()).boundingRect("...");
		cutting.translate(-cutting.topLeft()); // move topLeft to (0, 0)
		text.setWidth(width() - cutting.width());
		text.translate(QPoint(cutting.width(), 0));
		P.drawText(cutting, "...");
	}

	P.drawText(text, Qt::AlignRight, name);
}

void ImageView::resizeEvent(QResizeEvent * /* e */)
{
	if (image.height() == 0 || image.width() == 0) return;
	rect = geometry();
	rect.translate(-rect.topLeft());
	zoom(0); // update zoom
}

void ImageView::mouseDoubleClickEvent(QMouseEvent * /* e */)
{
	if (QApplication::keyboardModifiers() == Qt::ControlModifier)
		zoom(-1);
	else
		zoom(1);
}

void ImageView::mousePressEvent(QMouseEvent * e)
{
	if (e->button() & Qt::RightButton)
		QWidget::mousePressEvent(e);
	else mousePos = e->pos();
}
	
void ImageView::mouseMoveEvent(QMouseEvent * e)
{
	if (geometry().width() == 0 || geometry().height() == 0) return;
	area.translate((double)(mousePos.x() - e->pos().x()) / (double)rect.width() * area.width(),
		(double)(mousePos.y() - e->pos().y()) / (double)rect.height() * area.height());
	mousePos = e->pos();
	update();
}

void ImageView::wheelEvent(QWheelEvent *e)
{
	zoom(e->delta() / 8.0 / 360.0 * 10.0); // one wheel rotation is about 10 steps
}

void ImageView::zoom(double levelStep)
{
	if (name.isEmpty() || !rect.isValid())
		return;

	// zoomValue (in percent) increases/decreases following this function: 100 * sqrt(2)^x
	// round about it results in -> 100% 150% 200% 300% 400% 550% 800% (see zooming values e.g. in gimp)
	double newZoom = zoomLevel * pow(sqrt(2.0), levelStep);
	if (newZoom > 256 || newZoom < 0.8) // only zoom up to 25600 % or down to 80%
		return;

	QRectF oldArea = area;
	QPointF center = area.center();
	area.setWidth(1 / newZoom * image.width());
	area.setHeight(1 / newZoom * image.height());
	double rAspect = (double)rect.height() / (double)rect.width(); // ensure that area has the same aspect as rect has
	double aAspect = (double)area.height() / (double)area.width();
	if (rAspect > aAspect)
		area.setHeight(area.width() * rAspect);
	else if (rAspect < aAspect)
		area.setWidth(area.height() / rAspect);
	area.moveCenter(center);

	if (levelStep > 0 ) {
		QPointF cursor = mapFromGlobal(QCursor::pos());
		QPointF old = cursor / (double)rect.width() * oldArea.width() + oldArea.topLeft(); // map to image coordinates
		QPointF neu = cursor / (double)rect.width() * area.width() + area.topLeft(); // map to image coordinates
		area.translate(old - neu); // ensure that the point under cursor doesn't move
	}
	zoomLevel = newZoom;

	update();
}



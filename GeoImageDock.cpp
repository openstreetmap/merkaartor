#include "GeoImageDock.h"

#include "Map/TrackPoint.h"
#include "Map/MapLayer.h"
#include "Command/DocumentCommands.h"

#include <QtGui/QInputDialog>
#include <QtGui/QMessageBox>


GeoImageDock::GeoImageDock(MainWindow *aMain)
	: QDockWidget(aMain)
{
	curImage = -1;
	setWindowTitle(tr("Geo Images"));
	Image = new ImageView(this);
	setWidget(Image);
	setObjectName("geoImageDock");
}

GeoImageDock::~GeoImageDock(void)
{
	delete widget();
}

void GeoImageDock::setImage(int ImageId)
{
	if (ImageId == curImage)
		return;
	if (ImageId != -1 && Images.size() > ImageId) {
		Image->setImage(Images.at(ImageId));
		curImage = ImageId;
	}
	else {
		Image->setImage(QPair<QString, QPixmap>("", QPixmap()));
		curImage = -1;
	}
}


void GeoImageDock::loadImages(QStringList fileNames, MapDocument *theDocument, MapView *theView)
{
	QString file;
	QPixmap img;
	QDateTime time;
	bool foundTime;

	Exiv2::Image::AutoPtr image;
	Exiv2::ExifData *exifData;
	Exiv2::ExifData::const_iterator i, end;

	MapLayer *theLayer;
	{ // retrieve the target layer from the user
		QStringList layers;
		QList<int> layerId;
		unsigned int i;
		MapLayer *layer;
		for (i=0;i<theDocument->layerSize();i++) {
			layer = theDocument->getLayer(i);
			if (layer->className() == "TrackMapLayer" || layer->className() == "DrawingMapLayer") {
				layers.append(theDocument->getLayer(i)->name());
				layerId.append(i);
			}
		}

		if (layers.size() == 0) {
			QMessageBox::critical(this, tr("No layers"), tr("No suitable layer found. Please first download data from OSM server or open a track."));
			return;
		}

		bool ok;
		QString name = QInputDialog::getItem(NULL, tr("Load geotagged Images"),
		 tr("Select the layer to which the images belong:"), layers, 0, false, &ok);
		if (ok && !name.isEmpty())
			theLayer = theDocument->getLayer(layerId.at(layers.indexOf(name)));
		else
			return;
	}

	CommandList * theList = new CommandList(tr("Loading Images"));

	bool ok;
	int offset = -1;

	foreach(file, fileNames) {
		if (!img.load(file))
			WARNING("Image broken", "Cannot open image %1.");

		if (img.isNull())
			continue;

    	image = Exiv2::ImageFactory::open(file.toAscii().constData());
		if (image.get() == 0)
			WARNING("exiv2", "Error with exiv2 in %1.");
		image->readMetadata();

		exifData = &  image->exifData();
		if (exifData->empty())
			WARNING("exif missing", "No exif data in image %1.");
		end = exifData->end();
		foundTime = false;
		
		QString latS((*exifData)["Exif.GPSInfo.GPSLatitude"].toString().c_str());
		QString lonS((*exifData)["Exif.GPSInfo.GPSLongitude"].toString().c_str());
		QString timeStamp((*exifData)["Exif.Image.DateTime"].toString().c_str());

		if (!latS.isEmpty() && !lonS.isEmpty()) {
			double lat = 0.0, lon = 0.0, *cur;
			QString curS;
			int i;
			curS = latS;
			cur = &lat;
			for (i=0;i<=1;i++) { // parse latS and lonS. format: "h/d m/d s/d" (with d as divider)
				QList<int> p;
				p.append(curS.indexOf("/"));
				p.append(curS.indexOf(" ", p.last()));
				p.append(curS.indexOf("/", p.last()));
				p.append(curS.indexOf(" ", p.last()));
				p.append(curS.indexOf("/", p.last()));
				p.append(curS.indexOf(" ", p.last()));

				*cur = (double)curS.left(p.at(0)).toInt() / (double)curS.mid(p.at(0)+1, p.at(1)-p.at(0)-1).toInt() + // hours
					(double)curS.mid(p.at(1)+1, p.at(2)-p.at(1)-1).toInt() / (double)curS.mid(p.at(2)+1, p.at(3)-p.at(2)-1).toInt() / 60.0 + // minutes
					(double)curS.mid(p.at(3)+1, p.at(4)-p.at(3)-1).toInt() / (double)curS.mid(p.at(4)+1, p.at(5)-p.at(4)-1).toInt() / 60.0 / 60.0; // seconds
					
				curS = lonS;
				cur = &lon;
			}
			TrackPoint *Pt = new TrackPoint( Coord( angToInt(lat), angToInt(lon)));
			Pt->setTag("Picture", "GeoTagged"); // Is this the nicest way to avoid the "?"-Image for this trackpoint?
			Pt->setImageId(Images.size());
			Images.append(QPair<QString, QPixmap>(file, img));
			theList->add(new AddFeatureCommand(theLayer, Pt, false));
		} else if (!timeStamp.isEmpty()) {
	
			if (offset == -1) { // ask the user to specify an offset for the images
				offset = QInputDialog::getInteger( NULL, tr("Set an offset"),
				 tr("Specify an offset\n(positive values will position the\nimages more to the end of the track):"),
				 0, 0, (int) ((unsigned int)-1 / 2), 1, &ok);
				if (!ok) return;
			}

			time = QDateTime::fromString(timeStamp, "yyyy:MM:dd hh:mm:ss").addSecs(offset);

			MapFeature *feature = NULL, *bestFeature = NULL;
			TrackPoint *Pt;
			unsigned int secondsTo = (unsigned int)-1, a;
			unsigned int u;

			for (u=0; u<theLayer->size(); feature = theLayer->get(u++)) {
				if (!feature) { qDebug() << "is this a bug???"; continue; } // TODO
				if ((Pt = dynamic_cast<TrackPoint*>(feature))) {
					a = time.secsTo(Pt->time());
					if (a < secondsTo) {
						secondsTo = a;
						bestFeature = feature;
					}
				}
			}

			if (!bestFeature)
				WARNING("No TrackPoints", "No TrackPoints found for image %1");

			if (secondsTo >= 15)
				if ( QMessageBox::question(NULL, tr("Wrong image?"),
				 tr("Image \"%1\" was taken %2 seconds before the next trackpoint was recorded.\n" \
				 "Do you still want to use it?").arg(file).arg(secondsTo), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes)
					continue;

			if (!(Pt = dynamic_cast<TrackPoint*>(bestFeature)))
				WARNING("Bug?", "Can't access TrackPoint!\n(%1)");

			Pt->setImageId(Images.size());
			Images.append(QPair<QString, QPixmap>(file, img));

		} else
			WARNING("No geo informations", "Image %1 is not a geotagged image.");
	}

	theView->invalidate(true, false);
	if (theList->size()) {
		theDocument->addHistory(theList);
	}
	else
		delete theList;

}

bool GeoImageDock::warning(QString title, QString message)
{
	if ( QMessageBox::warning ( NULL, title, message, QMessageBox::Ignore | QMessageBox::Cancel, QMessageBox::Ignore ) == QMessageBox::Ignore)
		return true;
	else
		return false;
}


// *** ImageView *** //

ImageView::ImageView(QWidget *parent)
	: QWidget(parent)
{
}

ImageView::~ImageView()
{
}

void ImageView::setImage(QPair<QString, QPixmap> img)
{
	name = img.first;
	image = img.second;
	area = QRect(QPoint(0, 0), image.size());
	resizeEvent(NULL);
	update();
}

void ImageView::paintEvent(QPaintEvent * /* e */)
{
	QPainter P(this);

	P.drawPixmap(rect, image.copy(area));

	QFontMetrics metrics(P.font());
	QRect text = metrics.boundingRect(name);
	text.translate(-text.topLeft());
	P.fillRect(text, QColor(255, 255, 255));
	P.drawText(text, name);
}

void ImageView::resizeEvent(QResizeEvent * /* e */)
{
	if (image.height() == 0 || image.width() == 0) return;
	rect = geometry();
	rect.translate(-rect.topLeft());
	aspect = (double)image.height() / (double)image.width();

	if (aspect * (double)rect.width() > rect.height()) rect.setWidth((int)((double)rect.height() / aspect));
	else rect.setHeight((int)((double)rect.width() * aspect));
}

void ImageView::mouseDoubleClickEvent(QMouseEvent * e)
{
	if (e->button() & Qt::LeftButton) zoom((int)(0.0652 * image.width()));
}

void ImageView::mousePressEvent(QMouseEvent * e)
{
	if (e->button() & Qt::RightButton) zoom((int)(-0.0652 * image.width()));
	else mousePos = e->pos();
}
	
void ImageView::mouseMoveEvent(QMouseEvent * e)
{
	if (geometry().width() == 0 || geometry().height() == 0) return;
	area.translate(QPoint((int)((double)(mousePos.x() - e->pos().x()) / (double)geometry().width() * (double)area.width()),
		(int)((double)(mousePos.y() - e->pos().y()) / (double)geometry().height() * (double)area.height())));
	mousePos = e->pos();
	update();
}

void ImageView::wheelEvent(QWheelEvent *e)
{
	zoom((int)((double)e->delta() / 8.0 / 360.0 * (double)image.width()));
}

void ImageView::zoom(int level)
{
	QPoint zoomValue(level, (int)((double)level * aspect));
	area = QRect(area.topLeft() + zoomValue, area.bottomRight() - zoomValue);
	if (level > 0 ) {
		QPoint cursor = mapFromGlobal(QCursor::pos());
		area.translate((int)(((double)cursor.x() - (double)rect.width() / 2.0) / (double)rect.width() * (double)zoomValue.x() * 2),
			(int)(((double)cursor.y() - (double)rect.height() / 2.0) / (double)rect.height() * (double)zoomValue.y() * 2) );
	}
	update();
}




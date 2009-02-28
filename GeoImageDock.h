
#include "MapView.h"
#include "MainWindow.h"
#include "Map/MapDocument.h"

#include <QtGui/QPainter>
#include <QtGui/QDockWidget>
#include <QtGui/QMouseEvent>
#include <QtGui/QShortcut>
#include <exiv2/image.hpp>
#include <exiv2/exif.hpp>


#define WARNING(title, message) { \
	if (QMessageBox::warning(this, title, message.arg(file), \
	 QMessageBox::Ignore | QMessageBox::Cancel, QMessageBox::Ignore) == QMessageBox::Ignore) \
		continue; \
	else { \
		theView->invalidate(true, false); \
		return; \
	} \
}

#define QUESTION(title, message, always) { \
	if (always == 0) { \
		int replyButton = QMessageBox::question(this, title, message, \
		 QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Abort, QMessageBox::Yes ); \
		if (replyButton == QMessageBox::No) \
			continue; \
		else if (replyButton == QMessageBox::Abort) { \
			theView->invalidate(true, false); \
			return; \
		} \
		else if (replyButton != QMessageBox::Yes) \
			always = replyButton; \
	} \
	if (always == QMessageBox::NoToAll) \
		continue; \
}
	
class ImageView;

class GeoImageDock : public QDockWidget
{
	Q_OBJECT

public:
	GeoImageDock(MainWindow *aMain);
	~GeoImageDock(void);

	void loadImages(QStringList fileNames);
	void setImage(TrackPoint *Pt);

	void addGeoDataToImage(Coord pos, const QString & file);

private slots:
	void removeImages(void);
	void toClipboard(void);

private:
	QStringList Images;
	int curImage;

	ImageView *Image;

	// first: TrackPoint::id(); second.first: image filename; second.second: true if we have to delete the TrackPoint
	QList<QPair<QString, QPair<QString, bool> > > usedTrackPoints;
	MainWindow *Main;

};

class ImageView : public QWidget
{
public:
	ImageView(QWidget *parent);
	~ImageView();

	void setImage(QString filename);

protected:
	void paintEvent(QPaintEvent *e);
	void mousePressEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void wheelEvent(QWheelEvent *e);
	void mouseDoubleClickEvent(QMouseEvent *e);
	void resizeEvent(QResizeEvent *e);

private:
	QImage image;
	QString name;
	QPoint mousePos;
	QRect rect;
	QRectF area;

	double zoomLevel; // zoom in percent

	void zoom(double levelStep); // zoom levelStep steps

};

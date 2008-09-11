
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
	if (QMessageBox::warning(this, tr(title), tr(message).arg(file), \
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

	void loadImages(QStringList fileNames, MapDocument *theDocument, MapView *theView);
	void setImage(int ImageId);

private slots:
	void removeImages(void);
	void toClipboard(void);

private:
	QStringList Images;
	int curImage;

	ImageView *Image;

	QList<MapLayer*> activeLayers;
	MapView *theView;

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
	QPixmap image;
	QString name;
	QPoint mousePos;
	QRect area, rect;

	double aspect;
	
	void zoom(int level);

};


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
	if (warning(tr(title), tr(message).arg(file))) \
		continue; \
	else \
		 return; }

class ImageView;

class GeoImageDock : public QDockWidget
{
	Q_OBJECT
public:
	GeoImageDock(MainWindow *aMain);
	~GeoImageDock(void);

	void loadImages(QStringList fileNames, MapDocument *theDocument, MapView *theView);
	void setImage(int ImageId);

protected:
	//void resizeEvent(QResizeEvent *e);

private:
	QList<QPair<QString, QPixmap> > Images;
	int curImage;

	ImageView *Image;

	bool warning(QString title, QString message);
};

class ImageView : public QWidget
{
public:
	ImageView(QWidget *parent);
	~ImageView();

	void setImage(QPair<QString, QPixmap> img);

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

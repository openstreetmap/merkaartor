#ifndef MERKATOR_IMPORTGPX_H_
#define MERKATOR_IMPORTGPX_H_

#include <QList>
#include <QObject>
#include <QFlags>

class Document;
class TrackLayer;

class QByteArray;
class QString;
class QWidget;

class ImportGPX : public QObject {
    Q_OBJECT
    public:
    enum class Option {
        NoOptions = 0x0,
        MakeSegmented = 0x1,
        DetectAnonymizedSegments = 0x2,
        ForceWaypoints = 0x4,
    };
    Q_DECLARE_FLAGS(Options,Option)

    static bool import(QWidget* aParent, const QString& aFilename, Document* theDocument, QList<TrackLayer*>& theTracklayers);
    static bool import(QWidget* aParent, QByteArray& aFile, Document* theDocument, QList<TrackLayer*>& theTracklayers, Options importOptions);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ImportGPX::Options)

#endif



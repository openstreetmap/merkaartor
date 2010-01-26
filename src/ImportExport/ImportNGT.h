#ifndef MERKAARTOR_IMPORTNGT_H_
#define MERKAARTOR_IMPORTNGT_H_

class Layer;
class Document;
class QString;
class QWidget;

bool importNGT(QWidget* aParent, const QString& aFilename, Document* theDocument, Layer* theLayer);

#endif


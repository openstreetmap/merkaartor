#ifndef MERKAARTOR_EXPORTOSM_H_
#define MERKAARTOR_EXPORTOSM_H_

class Relation;
class Way;
class Node;

#include <QtCore/QString>

QString exportOSM(const Node& Pt, const QString&  ChangesetId);
QString exportOSM(const Way& R, const QString&  ChangesetId);
QString exportOSM(const Relation& R, const QString&  ChangesetId);

QString wrapOSM(const QString& S, const QString& ChangeSetId);
const QString encodeAttributes(const QString & text);

#endif



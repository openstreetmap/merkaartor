#ifndef MERKAARTOR_EXPORTOSM_H_
#define MERKAARTOR_EXPORTOSM_H_

class Relation;
class Road;
class TrackPoint;

#include <QtCore/QString>

QString exportOSM(const TrackPoint& Pt);
QString exportOSM(const Road& R);
QString exportOSM(const Relation& R);

QString wrapOSM(const QString& S);

#endif



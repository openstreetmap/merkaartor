#ifndef MERKAARTOR_EXPORTOSM_H_
#define MERKAARTOR_EXPORTOSM_H_

class Road;
class TrackPoint;
class Way;

#include <QtCore/QString>

QString exportOSM(const TrackPoint& Pt);
QString exportOSM(const Way& W);
QString exportOSM(const Road& R);

QString wrapOSM(const QString& S);

#endif



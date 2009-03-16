#ifndef MERKATOR_SYNCOSM_H_
#define MERKATOR_SYNCOSM_H_

class MainWindow;

class QString;

void syncOSM(MainWindow* aMain, const QString& aWeb, const QString& aUser, const QString& aPwd, bool UseProxy, const QString& ProxyHost, int ProxyPort);

#endif



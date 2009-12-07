#include "Sync/SyncOSM.h"

#include "MainWindow.h"
#include "Command/Command.h"
#include "Maps/MapDocument.h"
#include "Maps/MapLayer.h"
#include "Maps/DownloadOSM.h"
#include "Sync/DirtyList.h"

#include <QtGui/QMessageBox>

void syncOSM(MainWindow* theMain, const QString& aWeb, const QString& aUser, const QString& aPwd)
{
	if (checkForConflicts(theMain->document()))
	{
		QMessageBox::warning(theMain,MainWindow::tr("Unresolved conflicts"), MainWindow::tr("Please resolve existing conflicts first"));
		return;
	}

	DirtyListBuild Future;
	theMain->document()->history().buildDirtyList(Future);
	DirtyListDescriber Describer(theMain->document(),Future);
	if (Describer.showChanges(theMain))
	{
		Future.resetUpdates();
		DirtyListExecutor Exec(theMain->document(),Future,aWeb,aUser,aPwd,Describer.tasks());
		Exec.executeChanges(theMain);

		if (M_PREFS->getAutoHistoryCleanup() && !theMain->document()->getDirtyOrOriginLayer()->getDirtySize())
			theMain->document()->history().cleanup();

		if (theMain->fileName != "") {
			if (MerkaartorPreferences::instance()->getAutoSaveDoc()) {
				theMain->saveDocument();
			} else {
				if (QMessageBox::warning(theMain,MainWindow::tr("Unsaved changes"), 
						MainWindow::tr("It is strongly recommended to save the changes to your document after an upload.\nDo you want to do this now?"),
						QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) {
					theMain->saveDocument();

				}
			}
		}
	}
}



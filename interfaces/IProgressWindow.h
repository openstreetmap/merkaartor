#ifndef MERKATOR_IPROGRESSWINDOW_H_
#define MERKATOR_IPROGRESSWINDOW_H_

class QProgressDialog;
class QProgressBar;
class QLabel;

class IProgressWindow
{
	protected:
		QProgressDialog*	theProgressDialog;
		QProgressBar*	theProgressBar;
		QLabel*			theProgressLabel;

	public:
		QProgressDialog* getProgressDialog() { return theProgressDialog; }
		QProgressBar* getProgressBar() { return theProgressBar; }
		QLabel*		  getProgressLabel() { return theProgressLabel; }
};

#endif

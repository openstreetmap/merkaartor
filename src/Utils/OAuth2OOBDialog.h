#ifndef __OAUTH2OOBDIALOG_H__
#define __OAUTH2OOBDIALOG_H__

#include <QDialog>
#include "ui_OAuth2OOBDialog.h"

class OAuth2OOBDialog : public QDialog
{
    Q_OBJECT

    public:
        OAuth2OOBDialog(QWidget *parent = 0);

        static QString getAuthCode(const QString url);

    private:
        Ui::OAuth2OOBDialog ui;
};

#endif

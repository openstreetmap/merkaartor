#include "OAuth2OOBDialog.h"

#include <QGuiApplication>
#include <QClipboard>
#include <QPushButton>
#include <QDesktopServices>
#include <QUrl>


OAuth2OOBDialog::OAuth2OOBDialog(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    connect(ui.btnCopy, &QPushButton::clicked, this, [this](){
        QGuiApplication::clipboard()->setText(ui.loginUrl->text());
    });

    connect(ui.btnOpen, &QPushButton::clicked, this, [this](){
        QDesktopServices::openUrl(QUrl(ui.loginUrl->text()));
    });
}

QString OAuth2OOBDialog::getAuthCode(const QString url) {
    OAuth2OOBDialog dialog;
    dialog.ui.loginUrl->setText(url);
    if (dialog.exec() == QDialog::Accepted) {
        return dialog.ui.loginCode->text();
    } else {
        return QString();
    }
}

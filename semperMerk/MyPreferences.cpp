//
// C++ Implementation: MyPreferences
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com> (C) 2008, 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "MyPreferences.h"

#include <QSettings>
#include <QToolButton>

#define PARAM_IMPLEMENT_BOOL(Param, Category, Default) \
    bool mb_##Category##Param = false; \
    void MyPreferences::set##Category##Param(bool theValue) \
    { \
        m_##Category##Param = theValue; \
        Sets->setValue(#Category"/"#Param, theValue); \
    } \
    bool MyPreferences::get##Category##Param() \
    { \
        if (!::mb_##Category##Param) { \
            ::mb_##Category##Param = true; \
            m_##Category##Param = Sets->value(#Category"/"#Param, Default).toBool(); \
        } \
        return  m_##Category##Param; \
    } \
    void MyPreferences::initWidget##Category##Param() \
    { \
        ui->w##Category##Param->setProperty("checked", get##Category##Param()); \
    } \
    void MyPreferences::retrieveWidget##Category##Param() \
    { \
        set##Category##Param(ui->w##Category##Param->property("checked").toBool()); \
    }

#define PARAM_IMPLEMENT_STRING(Param, Category, Default) \
    bool mb_##Category##Param = false; \
    void MyPreferences::set##Category##Param(QString theValue) \
    { \
        m_##Category##Param = theValue; \
        Sets->setValue(#Category"/"#Param, theValue); \
    } \
    QString MyPreferences::get##Category##Param() \
    { \
        if (!::mb_##Category##Param) { \
            ::mb_##Category##Param = true; \
            m_##Category##Param = Sets->value(#Category"/"#Param, Default).toString(); \
        } \
        return  m_##Category##Param; \
    } \
    void MyPreferences::initWidget##Category##Param() \
    { \
        if(QString(ui->w##Category##Param->metaObject()->className()) == "QLineEdit") \
            ui->w##Category##Param->setProperty("text", get##Category##Param()); \
        else \
        if(QString(ui->w##Category##Param->metaObject()->className()) == "QComboBox") \
            ui->w##Category##Param->setProperty("currentText", get##Category##Param()); \
    } \
    void MyPreferences::retrieveWidget##Category##Param() \
    { \
      if(QString(ui->w##Category##Param->metaObject()->className()) == "QLineEdit") \
        set##Category##Param(ui->w##Category##Param->property("text").toString()); \
      else \
      if(QString(ui->w##Category##Param->metaObject()->className()) == "QComboBox") \
        set##Category##Param(ui->w##Category##Param->property("currcurrentTextentIndex").toString()); \
    }

#define PARAM_IMPLEMENT_INT(Param, Category, Default) \
    bool mb_##Category##Param = false; \
    void MyPreferences::set##Category##Param(int theValue) \
    { \
        m_##Category##Param = theValue; \
        Sets->setValue(#Category"/"#Param, theValue); \
    } \
    int MyPreferences::get##Category##Param() \
    { \
        if (!::mb_##Category##Param) { \
            ::mb_##Category##Param = true; \
            m_##Category##Param = Sets->value(#Category"/"#Param, Default).toInt(); \
        } \
        return  m_##Category##Param; \
    } \
    void MyPreferences::initWidget##Category##Param() \
    { \
        if(QString(ui->w##Category##Param->metaObject()->className()) == "QComboBox") \
            ui->w##Category##Param->setProperty("currentIndex", get##Category##Param()); \
        else \
            ui->w##Category##Param->setProperty("value", get##Category##Param()); \
    } \
    void MyPreferences::retrieveWidget##Category##Param() \
    { \
        if(QString(ui->w##Category##Param->metaObject()->className()) == "QComboBox") \
            set##Category##Param(ui->w##Category##Param->property("currentIndex").toInt()); \
        else \
            set##Category##Param(ui->w##Category##Param->property("value").toInt()); \
    }

#define PARAM_IMPLEMENT_REAL(Param, Category, Default) \
    bool mb_##Category##Param = false; \
    void MyPreferences::set##Category##Param(qreal theValue) \
    { \
        m_##Category##Param = theValue; \
        Sets->setValue(#Category"/"#Param, theValue); \
    } \
qreal MyPreferences::get##Category##Param() \
    { \
        if (!::mb_##Category##Param) { \
            ::mb_##Category##Param = true; \
            m_##Category##Param = Sets->value(#Category"/"#Param, Default).toDouble(); \
        } \
        return  m_##Category##Param; \
    } \
    void MyPreferences::initWidget##Category##Param() \
    { \
        ui->w##Category##Param->setProperty("value", get##Category##Param()); \
    } \
    void MyPreferences::retrieveWidget##Category##Param() \
    { \
        set##Category##Param(ui->w##Category##Param->property("value").toDouble()); \
    }


/***************************/

#include "ui_MyPreferences.h"

MyPreferences* MyPreferences::m_prefInstance = 0;

MyPreferences::MyPreferences(QWidget *parent)
    :   QWidget(parent),
        ui(new Ui::MyPreferencesDialog)
{
    ui->setupUi(this);
    ui->dummy->setVisible(false);

    QSize btSz(32, 32);
    QToolButton* btOk = new QToolButton(this);
    btOk->setIcon(QIcon::fromTheme("go-down", QPixmap(":icons/down")));
    btOk->setIconSize(btSz);
    QToolButton* btExit = new QToolButton(this);
    btExit->setIcon(QIcon::fromTheme("application-exit", QPixmap(":icons/exit")));
    btExit->setIconSize(btSz);

    ui->buttonBox->addButton(btOk, QDialogButtonBox::AcceptRole);
    ui->buttonBox->addButton(btExit, QDialogButtonBox::RejectRole);

    Sets = new QSettings();
}

MyPreferences::~MyPreferences()
{
    delete Sets;
    delete ui;
}

void MyPreferences::accept()
{
    m_dialogResult = QDialog::Accepted;
}

void MyPreferences::reject()
{
    m_dialogResult = QDialog::Rejected;
}

bool MyPreferences::execPreferences()
{
    QEventLoop loop;
    connect(ui->buttonBox, SIGNAL(accepted()), &loop, SLOT(quit()));
    connect(ui->buttonBox, SIGNAL(rejected()), &loop, SLOT(quit()));

    loop.exec(QEventLoop::DialogExec);
    if (m_dialogResult == QDialog::Accepted) {
    } else {
        QCoreApplication::quit();
    }

    return (m_dialogResult == QDialog::Accepted);
}

void MyPreferences::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}



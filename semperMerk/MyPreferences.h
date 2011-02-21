//
// C++ Interface: MyPreferences
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com> (C) 2008, 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef MYPREFERENCES_H
#define MYPREFERENCES_H

#include <QDialog>

class QSettings;

#define PARAM_DECLARE_BOOL(Param, Category) \
    private: \
        bool m_##Category##Param; \
        void initWidget##Category##Param(); \
        void retrieveWidget##Category##Param(); \
    public slots: \
        void set##Category##Param(bool theValue); \
    public: \
        bool get##Category##Param();

#define PARAM_DECLARE_STRING(Param, Category) \
    private: \
        QString m_##Category##Param; \
        void initWidget##Category##Param(); \
        void retrieveWidget##Category##Param(); \
    public slots: \
        void set##Category##Param(QString theValue); \
    public: \
        QString get##Category##Param();

#define PARAM_DECLARE_INT(Param, Category) \
    private: \
        int m_##Category##Param; \
        void initWidget##Category##Param(); \
        void retrieveWidget##Category##Param(); \
    public slots: \
        void set##Category##Param(int theValue); \
    public: \
        int get##Category##Param();

#define PARAM_DECLARE_REAL(Param, Category) \
    private: \
        qreal m_##Category##Param; \
        void initWidget##Category##Param(); \
        void retrieveWidget##Category##Param(); \
    public slots: \
        void set##Category##Param(qreal theValue); \
    public: \
        qreal get##Category##Param();

#define SAFE_DELETE(x) {delete (x); x = NULL;}
#define STRINGIFY(x) XSTRINGIFY(x)
#define XSTRINGIFY(x) #x

#define SEMPERMERK_PREFS MyPreferences::instance()

/***************************/

namespace Ui {
    class MyPreferencesDialog;
}

class MyPreferences : public QWidget
{
    Q_OBJECT

public:
    MyPreferences(QWidget *parent = 0);
    ~MyPreferences();

    static MyPreferences* instance() {
        if (!m_prefInstance) {
            m_prefInstance = new MyPreferences;
        }

        return m_prefInstance;
    }
    static MyPreferences* m_prefInstance;

public slots:
    bool execPreferences();

private slots:
    void accept();
    void reject();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::MyPreferencesDialog *ui;
    int m_dialogResult;

    QSettings * Sets;

public:
    PARAM_DECLARE_INT(UserAgent, General)
    PARAM_DECLARE_STRING(LastPicDownloadDir, General)
    PARAM_DECLARE_STRING(LastDataDownloadDir, General)
    PARAM_DECLARE_INT(InstantHistorySize, Caching)
    PARAM_DECLARE_BOOL(ControlAutoCollapse, Ui)
    PARAM_DECLARE_BOOL(BookmarkAutoBrowse, Ui)
};

#endif // MYPREFERENCES_H

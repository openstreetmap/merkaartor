#include "Global.h"

#include "TagSelectorWidget.h"
#include "ui_TagSelectorWidget.h"

#include <QCompleter>

#include "Document.h"

TagSelectorWidget::TagSelectorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TagSelectorWidget)
{
    ui->setupUi(this);

    ui->cbKey->setInsertPolicy(QComboBox::InsertAlphabetically);
    ui->cbValue->setInsertPolicy(QComboBox::InsertAlphabetically);

    QStringList ksl = g_getTagKeyList();
    QCompleter* completer = new QCompleter(ksl, (QObject *)this);
    ui->cbKey->insertItems(-1, ksl);
    completer->setCompletionMode(QCompleter::InlineCompletion);
    completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    ui->cbKey->setCompleter(completer);
    ui->cbKey->setEditable(true);

    ui->cbValue->insertItems(-1, g_getTagValueList("*"));
    ui->cbValue->setEditable(true);

    connect(ui->btAnd, SIGNAL(clicked()), this, SIGNAL(sigAnd()));
}

TagSelectorWidget::~TagSelectorWidget()
{
    delete ui;
}

void TagSelectorWidget::changeEvent(QEvent *e)
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

void TagSelectorWidget::on_cbKey_editTextChanged(const QString & text)
{
    ui->cbValue->clear();

    QStringList sl = g_getTagValueList(text);
    QCompleter* completer = new QCompleter(sl, (QObject *)this);
    ui->cbValue->insertItems(-1, g_getTagValueList(text));
    completer->setCompletionMode(QCompleter::InlineCompletion);
    completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    if (ui->cbValue->completer())
        delete ui->cbValue->completer();
    ui->cbValue->setCompleter(completer);
}


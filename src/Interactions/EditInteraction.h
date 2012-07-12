#ifndef MERKAARTOR_EDITINTERACTION_H_
#define MERKAARTOR_EDITINTERACTION_H_

#include "Interaction.h"
#include "SelectInteraction.h"

class MoveNodeInteraction;

class EditInteraction :	public SelectInteraction
{
    Q_OBJECT

public:
    EditInteraction();

    virtual QString toHtml();
#ifndef _MOBILE
    virtual QCursor cursor() const;
#endif

public slots:
    void on_remove_triggered();
    void on_reverse_triggered();
};

#endif



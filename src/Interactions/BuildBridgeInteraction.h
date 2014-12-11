#ifndef MERKAARTOR_MARKBRIDGEINTERATION_H_
#define MERKAARTOR_MARKBRIDGEINTERATION_H_

#include "Interaction.h"
#include "Feature.h"

class BuildBridgeInteraction : public FeatureSnapInteraction
{
    public:
        BuildBridgeInteraction(MainWindow* aMain);
        ~BuildBridgeInteraction(void);


        //virtual void snapMousePressEvent(QMouseEvent * event, Feature* aLast);
        virtual void snapMouseReleaseEvent(QMouseEvent * event, Feature* aLast);
        //virtual void snapMouseMoveEvent(QMouseEvent* event, Feature* aLast);
        virtual QString toHtml();
#ifndef _MOBILE
        virtual QCursor cursor() const;
#endif
        static Node* createNode(Coord P, Feature* aFeat);
		void splitAndMark();
		bool freshSplit;
};

#endif

#ifndef MERKATOR_TRACKSEGMENT_H_
#define MERKATOR_TRACKSEGMENT_H_

#include "Feature.h"

class TrackSegmentPrivate;
class Node;

class QProgressDialog;

class TrackSegment : public Feature
{
    public:
        TrackSegment(void);
        ~TrackSegment(void);
    private:
        TrackSegment(const TrackSegment& other);

        void drawDirectionMarkers(QPainter & P, QPen & pen, const QPointF & FromF, const QPointF & ToF);

    public:
        virtual QString getClass() const {return "TrackSegment";}
        virtual char getType() const {return IFeature::GpxSegment;}
        virtual void updateMeta();

        virtual const CoordBox& boundingBox(bool update=true) const;
        virtual void draw(QPainter& P, MapView* theView);
        virtual void drawSpecial(QPainter& P, QPen& Pen, MapView* theView);
        virtual void drawParentsSpecial(QPainter& P, QPen& Pen, MapView* theView);
        virtual void drawChildrenSpecial(QPainter& P, QPen& Pen, MapView* theView, int depth);

        virtual double pixelDistance(const QPointF& Target, double ClearEndDistance, bool selectNodes, MapView* theView) const;
        void cascadedRemoveIfUsing(Document* theDocument, Feature* aFeature, CommandList* theList, const QList<Feature*>& Alternatives);
        virtual bool notEverythingDownloaded();
        virtual QString description() const;

        void add(Node* aPoint);
        void add(Node* Pt, int Idx);
        virtual int find(Feature* Pt) const;
        virtual void remove(int idx);
        virtual void remove(Feature* F);
        virtual Feature* get(int idx);
        virtual int size() const;
        Node* getNode(int idx);
        virtual const Feature* get(int Idx) const;
        virtual bool isNull() const;

        void sortByTime();
        virtual void partChanged(Feature* F, int ChangeId);

        double distance();
        int duration() const;

        virtual bool toGPX(QXmlStreamWriter& stream, QProgressDialog * progress, bool forExport=false);
        static TrackSegment* fromGPX(Document* d, Layer* L, QXmlStreamReader& stream, QProgressDialog * progress);
        virtual bool toXML(QXmlStreamWriter& stream, QProgressDialog * progress, bool strict=false,QString changetsetid="");
        static TrackSegment* fromXML(Document* d, Layer* L, QXmlStreamReader& stream, QProgressDialog * progress);

        virtual QString toHtml() {return "";}

private:
        TrackSegmentPrivate* p;
};

#endif



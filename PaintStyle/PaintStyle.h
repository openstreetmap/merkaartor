#ifndef MERKAARTOR_PAINTSTYLE_H_
#define MERKAARTOR_PAINTSTYLE_H_

class Road;
class Way;

class PaintStyle
{
	public:
		virtual ~PaintStyle() = 0;

		virtual PaintStyle* firstLayer() = 0;
		virtual PaintStyle* nextLayer() = 0;
		virtual void draw(Way* W) = 0;
		virtual void draw(Road* R) = 0;
};

class CascadingPaintStyleLayer : public PaintStyle
{
	public:
		CascadingPaintStyleLayer();

		void setNextLayer(PaintStyle* n);
		virtual PaintStyle* firstLayer();
		virtual PaintStyle* nextLayer();
	private:
		PaintStyle* Next;
};

class EmptyPaintStyle : public PaintStyle
{
	public:
		virtual void draw(Way* W);
		virtual void draw(Road* R);
};

#endif
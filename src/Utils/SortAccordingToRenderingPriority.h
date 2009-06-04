#ifndef SORTACCORDINGTORENDERINGPRIORITY_H
#define SORTACCORDINGTORENDERINGPRIORITY_H

class MapFeature;

class SortAccordingToRenderingPriority
{
	public:
		SortAccordingToRenderingPriority(double aPixelPerM)
			: PixelPerM(aPixelPerM)
		{
		}
		bool operator()(MapFeature* A, MapFeature* B)
		{
			return A->renderPriority(PixelPerM) < B->renderPriority(PixelPerM);
		}

		double PixelPerM;
};

#endif // SORTACCORDINGTORENDERINGPRIORITY_H

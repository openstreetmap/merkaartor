#ifndef SORTACCORDINGTORENDERINGPRIORITY_H
#define SORTACCORDINGTORENDERINGPRIORITY_H

class MapFeature;

class SortAccordingToRenderingPriority
{
	public:
		SortAccordingToRenderingPriority()
		{
		}
		bool operator()(MapFeature* A, MapFeature* B)
		{
			return A->renderPriority() < B->renderPriority();
		}
};

#endif // SORTACCORDINGTORENDERINGPRIORITY_H

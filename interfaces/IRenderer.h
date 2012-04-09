#ifndef IRENDERER_H
#define IRENDERER_H

#include <QFlags>

class RendererOptions
{
public:
    enum RenderOption
    {
        None = 0x0,
        BackgroundVisible = 0x1,
        ForegroundVisible = 0x2,
        TouchupVisible = 0x4,
        NamesVisible = 0x8,
        PhotosVisible = 0x10,
        VirtualNodesVisible = 0x20,
        NodesVisible = 0x40,
        TrackSegmentVisible = 0x80,
        RelationsVisible = 0x100,
        MapBackgroundVisible = 0x200,
        DownloadedVisible = 0x400,
        DirtyVisible = 0x800,
        ScaleVisible = 0x1000,
        LatLonGridVisible = 0x2000,
        UnstyledHidden = 0x4000,
        Interacting = 0x8000,
        LockZoom = 0x10000,
        ForPrinting = 0x20000,
        PrintAllLabels = 0x40000
    };
    Q_DECLARE_FLAGS(RenderOptions, RenderOption)

    enum DirectionalArrowsShowOption {
        ArrowsNever = 0x0,
        ArrowsOneway = 0x1,
        ArrowsAlways = 0x2
    };
    Q_DECLARE_FLAGS(DirectionalArrowsShowOptions, DirectionalArrowsShowOption)

    RenderOptions options;
    DirectionalArrowsShowOptions arrowOptions;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(RendererOptions::RenderOptions)
Q_DECLARE_OPERATORS_FOR_FLAGS(RendererOptions::DirectionalArrowsShowOptions)

#endif // IRENDERER_H

# Rendering in Merkaartor

Note: This document is work in progress and might contain mistakes and/or
old/partial information. Feel free to improve it.

## Rendering OSM

The OSM data is represented by a Document object. A view to this document is
represented by a MapView object. A single Document can be viewed through
multiple MapView objects, for example the main canvas and a printed map
(NativeRenderDialog).

MapView creates and OsmLayerRender object, which handles parallelization of
rendering. It splits the viewport into tiles and manages the rendering and
rendered pieces. For the rendering itself, QtConcurrent module is used and
executes MapRenderer to do the actual painting. 

Note that during painting, the map data must not be modified. This includes
various auxilary structures in the Document, like cached Painters (defined by
the current style).

We currently handle this by a few locks around some random places, which is
pretty fragile. Locking the document object would probably be the best way to
do it.



INCLUDEPATH += $$MERKAARTOR_SRC_DIR/Interactions
DEPENDPATH += $$MERKAARTOR_SRC_DIR/Interactions

HEADERS += \
    CreateAreaInteraction.h \
    CreateDoubleWayInteraction.h \
    CreateNodeInteraction.h \
    CreateRoundaboutInteraction.h \
    CreatePolygonInteraction.h \
    CreateSingleWayInteraction.h \
    Interaction.h \
    SelectInteraction.h \
    EditInteraction.h \
    MoveNodeInteraction.h \
    RotateInteraction.h \
    ScaleInteraction.h \
    ZoomInteraction.h \
    ExtrudeInteraction.h

SOURCES += \
    CreateAreaInteraction.cpp \
    CreateDoubleWayInteraction.cpp \
    CreateNodeInteraction.cpp \
    CreateSingleWayInteraction.cpp \
    CreateRoundaboutInteraction.cpp \
    CreatePolygonInteraction.cpp \
    Interaction.cpp \
    SelectInteraction.cpp \
    EditInteraction.cpp \
    MoveNodeInteraction.cpp \
    RotateInteraction.cpp \
    ScaleInteraction.cpp \
    ZoomInteraction.cpp \
    ExtrudeInteraction.cpp

FORMS +=  \
    CreateDoubleWayDock.ui \
    CreateRoundaboutDock.ui \

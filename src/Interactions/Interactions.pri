INCLUDEPATH += Interactions
DEPENDPATH += Interactions

HEADERS += \
	CreateAreaInteraction.h \
	CreateDoubleWayInteraction.h \
	CreateNodeInteraction.h \
	CreateRoundaboutInteraction.h \
	CreatePolygonInteraction.h \
	CreateSingleWayInteraction.h \
	EditInteraction.h \
	Interaction.h \
	MoveNodeInteraction.h \
	RotateInteraction.h \
	ZoomInteraction.h \

SOURCES += \
	CreateAreaInteraction.cpp \
	CreateDoubleWayInteraction.cpp \
	CreateNodeInteraction.cpp \
	CreateSingleWayInteraction.cpp \
	CreateRoundaboutInteraction.cpp \
	CreatePolygonInteraction.cpp \
	EditInteraction.cpp \
	Interaction.cpp \
	MoveNodeInteraction.cpp \
	RotateInteraction.cpp \
	ZoomInteraction.cpp \

FORMS +=  \
	CreateDoubleWayDock.ui \
	CreateRoundaboutDock.ui \

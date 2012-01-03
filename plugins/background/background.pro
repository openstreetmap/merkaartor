TEMPLATE = subdirs

#Qt Version
QT_VERSION = $$[QT_VERSION]
QT_VERSION = $$split(QT_VERSION, ".")
QT_VER_MAJ = $$member(QT_VERSION, 0)
QT_VER_MIN = $$member(QT_VERSION, 1)

!symbian {
    SUBDIRS += \
        # MArbitraryRasterMapBackground \
        # MNavitBackground \
#        MYahooTiledBackground \
#        MYahooBackground \
        MMsBingMapBackground \
        MWalkingPapersBackground

    greaterThan(QT_VER_MAJ, 3) : greaterThan(QT_VER_MIN, 5) {
        SUBDIRS += MCadastreFranceBackground
    }

    SUBDIRS += MGeoTiffBackground \
               MGdalBackground

    contains (SPATIALITE, 1) {
        SUBDIRS += MSpatialiteBackground
    }

}

